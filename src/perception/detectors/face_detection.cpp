#include "face_detection.h"

#include <thread>
#include <atomic>

#include "face_analysis.h"
#include <common/RingBuffer.h>

#include <opencv2/opencv.hpp>


const uint32_t LEARNING_INTERVAL = 10;
const uint32_t LEARNING_BUFFER_SIZE = 8;

namespace FaceDetection {

    struct Context {
        RingBuffer<FrameSet> frame_buffer;
        RingBuffer<FacePercept> percept_buffer;
        std::atomic<bool> shutdown_flag;
        std::thread worker;
        std::mutex drawing_mutex;

        std::atomic<bool> is_learning;
        std::atomic<bool> done_learn;

        std::vector<FacePercept> drawings;

        std::string learn_name;
        int learn_id;

        cv::Mat learning_buffer[LEARNING_BUFFER_SIZE];
        bool learning_buffer_valid[LEARNING_BUFFER_SIZE];
        size_t next_learning_buffer_id = 0;

        int next_person_id = 1;
        std::unordered_map<int, std::string> person_names;

        Context() : frame_buffer(4), percept_buffer(4), shutdown_flag(false) {}
    };

    Context* ctx;


    void initialize() {

        ctx = new Context();

        FaceAnalysis::initialize();

        ctx->worker = std::thread([]() {

            load_known_persons();

            uint32_t counter = 0;

            for (size_t i = 0; i < LEARNING_BUFFER_SIZE; ++i) {
                ctx->learning_buffer_valid[i] = false;
            }

            ctx->is_learning = false;
            ctx->done_learn = false;
            ctx->next_learning_buffer_id = 0;

            std::vector <FacePercept> percepts;
            while (!ctx->shutdown_flag) {

                if (ctx->done_learn) {
                    ctx->person_names.insert({ctx->learn_id, ctx->learn_name});
                    auto folder_name = FACE_FOLDER + "/" + std::to_string(ctx->learn_id) + ":" + ctx->learn_name;
                    fs::create_directory(folder_name);
                    for (size_t i = 0; i < LEARNING_BUFFER_SIZE; ++i) {
                        if (ctx->learning_buffer_valid[i]) {
                            FaceAnalysis::add_person_image(ctx->learn_id, ctx->learning_buffer[i]);
                            cv::imwrite(folder_name + "/" + std::to_string(i) + ".jpg", ctx->learning_buffer[i]);
                        }
                        ctx->learning_buffer_valid[i] = false;
                    }
                }

                auto frames = ctx->frame_buffer.take();
                FaceAnalysis::detect(frames, percepts);

                if (ctx->is_learning && counter++ % LEARNING_INTERVAL == 0 && !percepts.empty()) {
                    ctx->learning_buffer[ctx->next_learning_buffer_id] = *frames.colour;
                    ctx->learning_buffer_valid[ctx->next_learning_buffer_id] = true;
                    ctx->next_learning_buffer_id = (ctx->next_learning_buffer_id + 1) % LEARNING_BUFFER_SIZE;
                }

                {
                    std::lock_guard guard(ctx->drawing_mutex);
                    ctx->drawings.clear();
                }
                for (auto &p : percepts) {
                    if (p.id > 0) {
                        p.name = ctx->person_names.at(p.id);
                    }
                    ctx->percept_buffer.put(p);
                    std::lock_guard guard(ctx->drawing_mutex);
                    ctx->drawings.push_back(p);
                }
                percepts.clear();
            }
        });
    }

    void put(FrameSet frames) {
        ctx->frame_buffer.put(frames);
    }

    void take_all(std::vector<FacePercept>& percepts) {
        FacePercept p;
        while (ctx->percept_buffer.try_take(&p)) {
            percepts.push_back(p);
        }
    }

    std::vector<FacePercept> drawings() {
        std::lock_guard guard(ctx->drawing_mutex);
        std::vector<FacePercept> copy = ctx->drawings;
        return copy;
    }

    void shutdown() {
        ctx->shutdown_flag = true;
        ctx->worker.join();
        FaceAnalysis::shutdown();
        delete ctx;
    }

    void start_learn() {
        ctx->is_learning = true;
    }

    void finish_learn(std::string name) {
        ctx->learn_name = name;
        ctx->learn_id = ctx->next_person_id++;
        ctx->is_learning = false;
        ctx->done_learn = true;
    }

    void delete_person(std::string name) {

    }
}
