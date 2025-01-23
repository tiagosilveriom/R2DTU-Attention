//
// Created by s154250 on 9/26/19.
//

#include <opencv2/opencv.hpp>
#include "person_tracker.h"
#include <glm/gtx/norm.hpp>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <experimental/filesystem>
#include <glm/gtx/string_cast.hpp>

#include "../detectors/face_analysis.h"
#include "../detectors/skeleton_detection.h"

namespace fs = std::experimental::filesystem;

namespace PersonTracker {

    static const int32_t LOSE_COUNTER = 30;
    static const int32_t LOCK_COUNTER = 30;
    static const int32_t SIGHTINGS_TO_LOCK = 5;
    static const int32_t LEARN_FACE_EMBEDDINGS_COUNT = 10;
    static const float SKELETON_MATCH_THRESHOLD = 0.7;
    static const float SIGMA_1 = 1.0;
    static const float SIGMA_2 = 1.0;
    static const float LAMBDA = 1.0;
    static const float FACE_DISTANCE_THRESHOLD = 0.15;
    static const uint32_t LEARNING_BUFFER_SIZE = 8;
    static const uint32_t LEARNING_INTERVAL = 5;

    struct KnownPerson {
        std::string name = "";
        bool priority = false;
        std::vector<FaceEmbedding*> face_embeddings;
    };

    struct Person {
        SkeletonPercept latest_skeleton;
        std::string name;
        uint32_t id;
        bool should_delete;

        int32_t timeout_counter;
        int32_t sightings_counter;
        bool hypothesis;
    };

    struct PersonTrackerState {
        std::vector<Person> persons;
        std::vector<KnownPerson> known_persons;
        uint32_t next_id = 1;
        std::vector<FaceEmbedding*> learning_buffer;
        size_t next_learning_buffer_id;
        uint32_t learning_id = 0;
    };

    static PersonTrackerState* state;

    const std::string FACE_FOLDER = "../data/faces";

    void load_known_persons() {
        for (auto& person : fs::directory_iterator(FACE_FOLDER)) {
            auto folder_name = std::string(person.path().filename());

            KnownPerson known;
            known.name = folder_name;
            known.priority = true;

            for (auto& img_path : fs::directory_iterator(person)) {
                auto img = cv::imread(std::string(img_path.path()));
                auto embedding = FaceAnalysis::detect_single(img);
                if (embedding) {
                    known.face_embeddings.push_back(embedding);
                }
            }
            state->known_persons.push_back(known);
        }
    }

    void start_learn(uint32_t id) {
        for (auto i = 0u; i < LEARNING_BUFFER_SIZE; ++i) {
            state->learning_buffer[i] = nullptr;
        }
        state->next_learning_buffer_id = 0;
        state->learning_id = id;
    }

    void learn(std::string name) {
        for (auto& person : state->persons) {
            if (person.id == state->learning_id) {
                person.should_delete = true;
                break;
            }
        }
        KnownPerson new_know;
        for (auto& embed : state->learning_buffer) {
            if (embed) {
                printf("Push!\n");
                new_know.face_embeddings.push_back(embed);
            }
        }
        new_know.priority = false;
        new_know.name = name;
        state->known_persons.push_back(new_know);
        state->learning_id = 0;
    }

    void delete_person(std::string name) {
        for (auto iter = state->persons.begin(); iter != state->persons.end(); ) {
            if (iter->name == name) {
                iter = state->persons.erase(iter);
            } else {
                ++iter;
            }
        }
        for (auto iter = state->known_persons.begin(); iter != state->known_persons.end(); ) {
            if (iter->name == name) {
                iter = state->known_persons.erase(iter);
            } else {
                ++iter;
            }
        }
    }

    void initialize() {
        state = new PersonTrackerState();
        SkeletonDetection::initialize();
        FaceAnalysis::initialize();
        load_known_persons();
        state->learning_buffer.resize(LEARNING_BUFFER_SIZE, nullptr);
    }

    float pose_similarity(const SkeletonPercept& a, const SkeletonPercept& b) {
        //K = Soft match, H = Squared distance
        float H = 0.0f, K = 0.0f;
        int count = 0;
        for (auto i = 0u; i < POSE_JOINTS; ++i) {
            if (a.certainty[i] == 0 || b.certainty[i] == 0) continue;
            count++;
            auto delta = a.joints[i] - b.joints[i];
            if (abs(delta.x) < a.box_size.x && abs(delta.y) < a.box_size.y) {
                K += tanh(a.certainty[i] / SIGMA_1) * tanh(b.certainty[i] / SIGMA_1);
            }

            H += exp(- distance2(a.joints[i], b.joints[i])/ SIGMA_2);
        }

        auto sim = (K + LAMBDA * H) / count;

        return sim;
    }

    void update_person(Person& person, const SkeletonPercept& skeleton) {
        if (person.hypothesis) {
            person.sightings_counter++;
        } else {
            person.timeout_counter = LOSE_COUNTER;
        }
        person.latest_skeleton = skeleton;
    }

    std::string identify_person(FaceEmbedding* person_embed) {
        std::string best_known;
        float best_avg_dist = 0.55;
        for (auto& known : state->known_persons) {
            float accumulator = 0.0f;
            uint32_t accumulator_counter = 0;
            for (auto& known_embed : known.face_embeddings) {
                float score = FaceAnalysis::compare_embeddings(known_embed, person_embed);
                accumulator += score;
                accumulator_counter++;
            }
            float avg = accumulator / accumulator_counter;
            if (avg <= best_avg_dist) {
                best_known = known.name;
                best_avg_dist = avg;
            }
        }

        return best_known;
    }

    static void face_scan_person(FrameSet& frames, Person& person) {
        if (!person.name.empty())
            return;

        /*auto uncertain = person.latest_skeleton.certainty[15] < 0.8 || person.latest_skeleton.certainty[16] < 0.8;
        auto turned = person.latest_skeleton.raw_joints[15].x > person.latest_skeleton.raw_joints[0].x || person.latest_skeleton.raw_joints[16].x < person.latest_skeleton.raw_joints[0].x;

        if (uncertain || turned)
            return;*/

        auto right_eye = person.latest_skeleton.raw_joints[15];
        auto left_eye = person.latest_skeleton.raw_joints[16];

        cv::Rect roi;
        roi.x = right_eye.x - 60;
        roi.width = left_eye.x + 60 - roi.x;
        roi.y = (left_eye.y + right_eye.y) / 2 - 80;
        roi.height = (left_eye.y + right_eye.y) / 2 + 80 - roi.y;

        if (roi.width < 0 || roi.height < 0 || roi.x < 0 || roi.y < 0 || roi.x+roi.width >= frames.colour->cols || roi.y+roi.height >= frames.colour->rows) return;

        cv::Mat face_crop = (*frames.colour)(roi);

        auto result = FaceAnalysis::detect_single(face_crop);
        if (result) {
            std::string known_name = identify_person(result);
            if (!known_name.empty()) {
                person.name = known_name;
                printf("Identified as %u!\n", person.id);
                delete result;
            } else if (state->learning_id == person.id) {
                state->learning_buffer[state->next_learning_buffer_id] = result;
                state->next_learning_buffer_id = (state->next_learning_buffer_id + 1) % LEARNING_BUFFER_SIZE;
            } else {
                delete result;
            }
        }
    }

    struct MatchScore {
        float score;
        size_t object;
        size_t percept;
        bool invalid;
    };

    void update(FrameSet frames) {

        auto percepts = SkeletonDetection::detect(frames);

        std::vector<MatchScore> scores;
        scores.reserve(percepts.size() * 2);


        std::unordered_set<size_t> unused_percepts;
        for (auto j = 0u; j < percepts.size(); ++j) {
            unused_percepts.insert(j);
            for (auto i = 0u; i < state->persons.size(); ++i) {
                float score = pose_similarity(state->persons[i].latest_skeleton, percepts[j]);
                if (score > 0.0) {
                    scores.push_back(MatchScore {score, i, j, false});
                }
            }
        }

        std::sort(scores.begin(), scores.end(), [](MatchScore a, MatchScore b) {
            return a.score > b.score;
        });

        for (auto i = 0u; i < scores.size(); ++i) {
            if (scores[i].invalid) continue;
            if (scores[i].score < SKELETON_MATCH_THRESHOLD) break;

            auto p = scores[i].percept;
            auto o = scores[i].object;
            for (auto j = i+1; j < scores.size(); ++j) {
                if (scores[j].percept == p || scores[j].object == o) {
                    scores[j].invalid = true;
                }
            }
            unused_percepts.erase(p);
            Person& person = state->persons[o];
            update_person(person, percepts[p]);
            if (!person.hypothesis) {
                face_scan_person(frames, person);
            }
        }

        for (auto i : unused_percepts) {
            Person hypothesis;
            hypothesis.name = "";
            hypothesis.id = state->next_id++;
            hypothesis.latest_skeleton = percepts[i];
            hypothesis.timeout_counter = LOCK_COUNTER;
            hypothesis.sightings_counter = 0;
            hypothesis.hypothesis = true;
            hypothesis.should_delete = false;
            state->persons.push_back(hypothesis);
        }
    }

    static void tick(std::vector<PersonPercept>& percepts) {

        std::vector<std::string> already_published_names;

        for (auto iter = state->persons.begin(); iter != state->persons.end(); ) {
            auto& person = *iter;
            if (person.hypothesis && person.sightings_counter >= SIGHTINGS_TO_LOCK) {
                person.hypothesis = false;
                person.timeout_counter = LOSE_COUNTER;
            }

            if (!person.hypothesis) {
                PersonPercept percept;
                bool already_published = false;
                for (auto& n : already_published_names) {
                    if (n == person.name) {
                        already_published = true;
                        break;
                    }
                }
                if (!already_published) {
                    percept.id = person.id;
                    percept.name = person.name.c_str();
                    percept.skeleton = person.latest_skeleton;
                    percepts.push_back(percept);
                    if (person.name.c_str()) {
                        already_published_names.push_back(person.name);
                    }
                }
            }

            person.timeout_counter = std::max(person.timeout_counter - 1, 0);
            if (person.timeout_counter <= 0 || person.should_delete) {
                iter = state->persons.erase(iter);
            } else {
                ++iter;
            }
        }
    }

    void process(FrameSet frames, std::vector<PersonPercept>& person_percepts) {
        update(frames);
        tick(person_percepts);
    }

    void shutdown() {
        delete state;
    }

}


