#include "../state.h"
#include "Observing.h"
#include <l10n/FBT.h>
#include "../EntityHeadTracker.h"
#include <asr/ASR.h>

class ShowBoxSequence : public Sequence {
public:
    ShowBoxSequence(std::string sentence, uint32_t box_id)
    : sentence(std::move(sentence)), box_id(box_id), counter(0) {}

    bool tick(Robot& r) override {

    glm::vec3 marker_pos(0.0f);

        if (counter == 0) {
            r.toggle_breathing(false);

            if (!sentence.empty()) {
                r.say(sentence);
            }

        }
        if (counter == 5) {
            start = r.get_current_pos();
            for (auto obj : r.world.iter_movables()) {
                if (obj->marker_id == box_id) {
                    marker_pos = obj->center;
                    break;
                }
            }

            r.turn_torso_to(marker_pos); // body orientation to the box

            std::cout << "OLHAR : \n";
            r.look_at_box(marker_pos);  // Head tracks target while pointing
        }

        if (counter == 100)
        {
            r.point_at(marker_pos); // point for 45 seconds
            //r.look_at(marker_pos);  // Head tracks target while pointing
        }

        if (counter == 300)
        {  
            r.rest_arm_position(); // body orientation back to the starting position
        }
        if (counter == 330)
        {  
            r.turn_torso_to(start); // body orientation back to the starting position
            //r.look_at(start);  // Reset gaze to starting position
            return true;
        }
        // TO REMOVE WHEN POINTING ACHIEVED
        /*
            auto target = marker_pos + target_offset;
            r.move_to(target);
        }
        if (counter == 100) {
            r.push();
        }
        if (counter == 250) {
            r.move_to(start);
        }

        if (counter == 300) {
            r.toggle_breathing(true);
            return true;
        }*/
        ++counter;
        return false;
    }

private:
    std::string sentence;
    uint32_t box_id;
    int counter;
    glm::vec3 start;
};

class PushCubeSequence : public Sequence {
public:
    PushCubeSequence(uint32_t cube_id)
        : counter(0), cube_id(cube_id) {}

    bool tick(Robot& r) override {
        if (counter == 0) {
            r.world.pause(true);
            r.toggle_breathing(false);
        }
        if (counter == 5) {
            start = r.get_current_pos();
            glm::vec3 target_offset = {-0.20, 0.0, -0.25};
            glm::vec3 marker_pos(0.0f);
            for (auto obj : r.world.iter_movables()) {
                if (obj->marker_id == cube_id) {
                    marker_pos = obj->center;
                    printf("Found cube at %f %f %f\n", obj->center.x, obj->center.y, obj->center.z);
                    break;
                }
            }
            auto target = marker_pos + target_offset;
            r.move_to(target);
        }
        if (counter == 100) {
            r.fbt2_push();
        }
        if (counter == 250) {
            //r.say("Ti Hi!");
            r.move_to(start);
        }

        if (counter == 300) {
            r.toggle_breathing(true);
            r.world.pause(false);
            return true;
        }
        ++counter;
        return false;
    }

private:
    int counter;
    uint32_t cube_id;
    glm::vec3 start;
};

#if LANG_DANISH
std::string accepted_sentences[] = {
        "\"hvor er den røde klods?\"",
        "\"hvor er den blå klods?\"",
        "\"hvor er den grønne klods?\"",
        "\"hvor tror Lasse at den røde klods er?\"",
        "\"hvor tror Lasse at den blå klods er?\"",
        "\"hvor tror Lasse at den grønne klods er?\"",
        "\"hvor tror Lasse den røde klods er?\"",
        "\"hvor tror Lasse den blå klods er?\"",
        "\"hvor tror Lasse den grønne klods er?\"",
        "\"hvor tror Thomas at den røde klods er?\"",
        "\"hvor tror Thomas at den blå klods er?",
        "\"hvor tror Thomas at den grønne klods er?\"",
};
#else
std::string accepted_sentences[] = {
        "\"why did you tell me that?\"",
        "\"i have come for the red cube?\"",
        "\"i have come for the blue cube?\"",
        "\"i have come for the green cube?\"",
        "\"where is the red cube?\"",
        "\"where is the blue cube?\"",
        "\"where is the green cube?\"",
        "\"where does Hansen believe that the red cube is?\"",
        "\"where does Hansen believe that the blue cube is?\"",
        "\"where does Hansen believe that the green cube is?\"",
        "\"where does Hansen believe that the red cube?\"",
        "\"where does Hansen believe that the blue cube?\"",
        "\"where does Hansen believe that the green cube?\"",
        "\"where does Hansen believe the red cube is?\"",
        "\"where does Hansen believe the blue cube is?\"",
        "\"where does Hansen believe the green cube is?\"",
        "\"where does Hansen believe the red cube to be?\"",
        "\"where does Hansen believe the blue cube to be?\"",
        "\"where does Hansen believe the green cube to be?\"",
};
#endif


bool Observing::epistemic_speech_trigger(const std::string& s) {
    auto answer = epistemic.answer_question(s);

    if (answer.empty()) return false;

    robot->say(answer);
    robot->play_animation(get_random_speaking_animation());
    return true;
}

bool Observing::fetch_response_trigger(const std::string& s) {
    auto red = strstr(s.c_str(), L10N_RED) != nullptr;
    auto blue = strstr(s.c_str(), L10N_BLUE) != nullptr;
    auto green = strstr(s.c_str(), L10N_GREEN) != nullptr;
    if (red || blue || green) {
        current_sequence = std::make_unique<ShowBoxSequence>("", expect_fetch_response_box_id);
        expect_fetch_response_box_id = 0;
        return true;
    }
    return false;
}

bool Observing::goal_statement_trigger(const std::string& s) {
    auto x = strstr(s.c_str(), "i have come for the ");
    if (x == nullptr) return false;

    uint32_t q = 0;
    if (strstr(x, L10N_RED)) q = 1;
    else if (strstr(x, L10N_GREEN)) q = 2;
    else if (strstr(x, L10N_BLUE)) q = 3;
    else return false;

    auto cube_name = "marker-" + std::to_string(q);

    std::string agent_name;

    for (auto& p : robot->world.iter_persons()) {
        if (p->status == WorldObjectStatus::Tracked) agent_name = p->name;
    }

    if (agent_name.empty()) return false;

    inform_about_false_belief_cube(agent_name, cube_name);

    return true;
}

bool Observing::need_speech_trigger(const std::string& s) {
    auto x = strstr(s.c_str(), "jeg skal bruge ");
    if (x == nullptr) return false;
    uint32_t q = 0;
    if (strstr(x, "en")) q = 1;
    else if (strstr(x, "to")) q = 2;
    else if (strstr(x, "tre")) q = 3;
    else return false;

    std::string sentence;

    std::string agent_name;

    for (auto& p : robot->world.iter_persons()) {
        if (p->status == WorldObjectStatus::Tracked) agent_name = p->name;
    }

    if (agent_name.empty()) return false;

    sentence += "tag den ";
    for (auto& c : robot->world.iter_movables()) {
        if (q == 0) break;
        if (c->marker_id >= 100) continue;

        auto cube_name = c->name;
        auto q1 = make_predicate("in", make_constant(cube_name), make_variable("x"));

        auto res = epistemic.query(q1);
        if (res.empty()) continue;
        auto box = res[0];
        q--;
        sentence += cube_name + " klods ";
        auto q2 = make_not(make_believes(agent_name, make_predicate("in", make_constant(cube_name), make_constant(box))));
        if (epistemic.evaluate(q1)) {
            sentence += " den er i kasse " + box;
        }
        if (q == 1)  sentence += " og ";
    }
    robot->say(sentence);

    return true;
}

bool Observing::why_speech_trigger(const std::string& s) {
    if (s.find("hvorfor") == std::string::npos && s.find("why") == std::string::npos) return false;
    robot->say(explain_sentence);
    return true;
}

void Observing::fbt2_push_trigger(World& world) {
    int person_present = 0;
    for (auto& person : world.iter_persons()) {
        if (person->status == WorldObjectStatus::Tracked)  person_present++;
    }
    if (person_present == 0) return;

    float trigger_threshold = 0.2;

    for (auto& movable : world.iter_movables()) {
        if (movable->status != WorldObjectStatus::Tracked || movable->marker_id >= 100) continue;
        if (glm::length(movable->center) < trigger_threshold) {
            current_sequence = std::make_unique<PushCubeSequence>(movable->marker_id);
        }
    }
}


void Observing::start_listening() {
    printf("Listening!\n");
  auto text = cloud_listen();
  bool heard_accepted = false;
    for (auto& s : accepted_sentences) {
        if (text == s) {
            robot->hear(text);
            heard_accepted = true;
        }
    }
    if (!heard_accepted) {
        robot->send_to_terminal("hear: " + text);
    } else {
        robot->send_to_terminal("");
    }
}

void Observing::stop_listening() {
    auto text = robot->finish_recording();
    robot->log_info("Robot", "\"" + text + "\"");
    bool heard_accepted = false;
    for (auto& s : accepted_sentences) {
        if (text == s) {
            robot->hear(text);
            heard_accepted = true;
        }
    }
    if (!heard_accepted) {
        robot->send_to_terminal("hear: " + text);
    } else {
        robot->send_to_terminal("");
    }
    recording = false;
}

void Observing::terminal_cmd(void* usr, const TerminalCmdMsg* msg) {
    auto cmd = Messages::read_string(&msg->cmd_type);
    auto param = Messages::read_string(&msg->param);
    if (cmd == "hear:") {
        for (auto& trig : speech_triggers) {
            if (trig(param)) break;
        }
    }
    if (cmd == "blank") {
        epistemic.blank_views();
    }
    if (cmd == "sr") {
        start_listening();
    }
    if (cmd == "push") {
        robot->toggle_breathing(false);
        robot->push();
    }
    if (cmd == "h1") {
        robot->hear("blå klods");
    }
    if (cmd == "h2") {
        robot->hear("Lasse blå klods");
    }
}

void Observing::enter(Robot& r) {
    using namespace std::placeholders;
    //r.log_info("FBT", "Starting false belief task");
    epistemic.reset();
    r.world.clear();
    current_sequence = nullptr;
    r.toggle_breathing(true);
    robot = &r;

    r.world.add_transformer(WorldEvent::Type::NewObject, std::bind(&Observing::object_new_transformer, this, _1, _2));
    r.world.add_transformer(WorldEvent::Type::Appear, std::bind(&Observing::object_appear_transformer, this, _1, _2));
    r.world.add_transformer(WorldEvent::Type::Disappear, std::bind(&Observing::object_disappear_transformer, this, _1, _2));

    r.world.add_trigger({
        WorldObjectType::Person,
        WorldObjectType::Movable,
        std::bind(&Observing::intention_trigger, this, _1, _2)
    });

    r.world.add_trigger({
        WorldObjectType::Person,
        WorldObjectType::Movable,
        std::bind(&Observing::intention_trigger_complementary, this, _1, _2)
    });
    counter = 0;
    printf("Observing starting \n");
}

void Observing::leave(Robot& r) {
    r.world.clear();
}

void Observing::inform_about_false_belief_cube(const std::string& agent, const std::string& cube) {

    auto q1 = make_and(
            make_believes(agent,make_predicate("in", make_constant(cube), make_variable("x"))),
            make_not(make_predicate("in", make_constant(cube), make_variable("x"))));

    auto res = epistemic.query(q1);
    if (res.empty()) {
        return;
    }
    auto belief_box = res[0];
    auto q2 = make_predicate("in", make_constant(cube), make_variable("x"));
    res = epistemic.query(q2);

    bool knows_location = res.empty();

    bool should_inform = epistemic.plan(cube, knows_location);

    if (should_inform) {

        std::string other_name;
        for (auto& other_agent : robot->world.iter_persons()) {
            if (!other_agent->name.empty() && other_agent->name != agent) {
                other_name = other_agent->name;
            }
        }

        auto actual_box = res[0];
        auto number = stoi(actual_box.substr(actual_box.size() - 4));
        char buffer[128];
        std::string color;
        if (cube == "marker-1") {
            color = L10N_RED;
        } else if (cube == "marker-2") {
            color = L10N_GREEN;
        } else if (cube == "marker-3") {
            color = L10N_BLUE;
        }
        sprintf(buffer, L10N_EXPLAIN, other_name.c_str(), color.c_str(), number-1000, agent.c_str());
        explain_sentence = buffer;

        sprintf(buffer, L10N_ANNOUNCE_LOCATION, color.c_str(), std::to_string(number-1000).c_str());

        current_sequence = std::make_unique<ShowBoxSequence>(buffer, number);
        epistemic.take_out_of_container("R2DTU", cube, belief_box);
        epistemic.put_in_container("R2DTU", cube, actual_box);
    }
}

void Observing::inform_about_false_belief_box(const std::string& agent, const std::string& box) {

    auto q1 = make_and(
            make_believes(agent,make_predicate("in", make_variable("x"), make_constant(box))),
            make_not(make_predicate("in", make_variable("x"), make_constant(box))));

    auto res = epistemic.query(q1);
    if (res.empty()) { //when agent beliefs regarding box are correct
        return;
    }
    auto cube_name = res[0];
    auto q2 = make_predicate("in", make_constant(cube_name), make_variable("x"));
    res = epistemic.query(q2);

    bool knows_location = res.empty();

    bool should_inform = epistemic.plan(cube_name, knows_location);

    if (should_inform) {

        std::string other_name;
        for (auto& other_agent : robot->world.iter_persons()) {
            if (!other_agent->name.empty() && other_agent->name != agent) {
                other_name = other_agent->name;
            }
        }
        
        auto box_name = res[0];
        auto number = stoi(box_name.substr(box_name.size() - 4));
        char buffer[128];
        std::string color;
        if (cube_name == "marker-1") {
            color = L10N_RED;
        } else if (cube_name == "marker-2") {
            color = L10N_GREEN;
        } else if (cube_name == "marker-3") {
            color = L10N_BLUE;
        }
        sprintf(buffer, L10N_EXPLAIN, other_name.c_str(), color.c_str(), number-1000, agent.c_str());
        explain_sentence = buffer;

        sprintf(buffer, L10N_LOOKING_FOR, color.c_str(), std::to_string(number-1000).c_str());

        current_sequence = std::make_unique<ShowBoxSequence>(buffer, number);
        
        /* Still to replace by Bottom Up */
        epistemic.take_out_of_container("R2DTU", cube_name, box);
        epistemic.put_in_container("R2DTU", cube_name, box_name);
        
    }
}


void Observing::inform_about_false_belief_object(const std::string& agent, const std::string& obj_holded) {

    /* get matching object (affordances) from object */
    int pos= obj_holded.find("-");
    auto obj_holded_name= obj_holded.substr(0,pos);
    auto matching_obj_name= get_matching_affordances(obj_holded_name); //return matching affordance object (e.g : input is knife, returns apple)
    std::cout<<agent << " is holding " << obj_holded_name <<  ", so he might be lookign for "  << matching_obj_name << "\n";
    auto q1 = make_and(
            make_believes(agent,make_predicate("in", make_constant(matching_obj_name), make_variable("x"))),
            make_not(make_predicate("in", make_constant(matching_obj_name), make_variable("x"))));

    auto res = epistemic.query(q1);
    if (res.empty()) { // No false belief
    std::cout<<"RETURN : NO FALSE BELIEF\n";
        return;
    }
    auto box_fb = res[0];
    auto q2 = make_predicate("in", make_constant(matching_obj_name), make_variable("x"));
    res = epistemic.query(q2);

    std::cout<<"*********Desired box: "<< res[0] << "***********\n";
    bool knows_location = res.empty(); // Matching object not inside boxes

    bool should_inform = epistemic.plan(matching_obj_name, knows_location);

    if (should_inform) {
        std::cout<<"*********SHould inform ***********\n";
        std::string other_name;
        for (auto& other_agent : robot->world.iter_persons()) {
            if (!other_agent->name.empty() && other_agent->name != agent) { /*TODO: this implementation is considering only 2 agents environment*/
                other_name = other_agent->name;
            }
        }

        auto box_name = res[0];
        auto number = stoi(box_name.substr(box_name.size() - 4));
        char buffer[128];

        sprintf(buffer, L10N_EXPLAIN, other_name.c_str(), matching_obj_name.c_str() , number-1000, agent.c_str());
        explain_sentence = buffer;

        sprintf(buffer, L10N_OBJECT_LOOKING_FOR, matching_obj_name.c_str() , std::to_string(number-1000).c_str());

        current_sequence = std::make_unique<ShowBoxSequence>(buffer, number);
        //epistemic.take_out_of_container("R2DTU", matching_obj_name, box_fb);
        //epistemic.put_in_container("R2DTU", matching_obj_name, box_name);
        epistemic.direct_conscious_attention(agent, matching_obj_name, box_name);
    }
}


Option<WorldEvent> Observing::object_new_transformer(World& world, WorldEvent e) {
    if (e.obj->type == WorldObjectType::Movable) {
        auto marker_id = ((MovableObj*) e.obj)->marker_id;
        if (marker_id < 200) { // markers from 0 to 100 , objects from 100 to 200
            epistemic.add_object(EpistemicObjectType::Storable, e.obj->name);
        }  /*else if (marker_id < 200){
            int pos= e.obj->name.find("-");
            auto filtered_name = e.obj->name.substr(0,pos);
            epistemic.add_object(EpistemicObjectType::Storable, filtered_name);
        }*/
        else if (marker_id > 1000) {
            epistemic.add_object(EpistemicObjectType::Container, e.obj->name);
        } else if (marker_id == 100) {
            start_listening();
        }
    } else if (e.obj->type == WorldObjectType::Person && !e.obj->name.empty()) {
        epistemic.add_object(EpistemicObjectType::Agent, e.obj->name);
        epistemic.agent_appear(e.obj->name);
        person_info.insert({e.obj->name, {false}});
    }
    return Option<WorldEvent>();
}


Option<WorldEvent> Observing::object_appear_transformer(World& world, WorldEvent e) {
    if (e.obj->type == WorldObjectType::Person && !e.obj->name.empty()) {
        printf("Seen again new\n");
        epistemic.agent_appear(e.obj->name);
    } else if (e.obj->type == WorldObjectType::Movable) {

        auto search = current_container.find(e.obj->name);
        // If agent appears, it is checked if it was stored at a container
        if (search != current_container.end()) {
            epistemic.take_out_of_container("R2DTU", search->first, search->second);
            current_container.erase(search);
        }

        auto obj = (MovableObj*) e.obj;
        if (obj->marker_id == 100) {
            start_listening(); 
        }
    }
    return Option<WorldEvent>();
}

Option<WorldEvent> Observing::object_disappear_transformer(World& world, WorldEvent e) {
    printf("Disappear transformer %u\n", ((MovableObj*) e.obj)->marker_id);
    if (e.obj->type == WorldObjectType::Movable) {
        auto obj = (MovableObj*) e.obj;

        if (obj->marker_id == 100) {
            stop_listening();
        }
        
        if (obj->marker_id >= 200) return Option<WorldEvent>();
        for (auto& movable : world.iter_movables()) {
            if (movable->marker_id < 1000) continue; // If it is not a container, continue
            if (movable->status == WorldObjectStatus::Hypothetical) continue;

            glm::vec3 aabb_offset = {0.0f, -0.35f, 0.1f};
            glm::vec3 aabb_extent = {0.1f, 0.30f, 0.30f};
            glm::vec3 aabb_origin = movable->center + aabb_offset;

            bool inside = glm::all(glm::lessThanEqual(glm::abs(obj->center - aabb_origin), aabb_extent));

            if (inside) {
                current_container.insert({obj->name, movable->name});
                epistemic.put_in_container("R2DTU", obj->name, movable->name); //obj is cube . movable is container
            } else {
                printf("Not inside %s %f %f %f\n", movable->name.c_str(), movable->center.x, movable->center.y, movable->center.z);
            }
        }
    } else if (e.obj->type == WorldObjectType::Person && !e.obj->name.empty()) {
        epistemic.agent_disappear(e.obj->name);
        person_info[e.obj->name].turned_back = false;
    }
    return Option<WorldEvent>();
}

void Observing::intention_trigger(WorldObject* obj1, WorldObject* obj2) {
    PersonObj* person = (PersonObj*) obj1;
    MovableObj* movable = (MovableObj*) obj2;
    if (person->status != WorldObjectStatus::Tracked || person->name.empty()) return;
    if (movable->status == WorldObjectStatus::Hypothetical || movable->marker_id <= 200) return;

    auto left_valid = person->latest_pose.certainty[7] > 0.3;
    auto right_valid = person->latest_pose.certainty[4] > 0.3;

    glm::vec3 aabb_offset = {0.0f, -0.35f, 0.1f};
    glm::vec3 aabb_extent = {0.1f, 0.30f, 0.25f};
    glm::vec3 aabb_origin = movable->center + aabb_offset;

    bool right_inside = glm::all(glm::lessThanEqual(glm::abs(person->latest_pose.joints[4] - aabb_origin), aabb_extent));
    bool left_inside = glm::all(glm::lessThanEqual(glm::abs(person->latest_pose.joints[7] - aabb_origin), aabb_extent));
    if (right_inside && right_valid) {
        inform_about_false_belief_box(person->name, movable->name);
    } else if (left_inside && left_valid) {
        inform_about_false_belief_box(person->name, movable->name);
    }

}

void Observing::intention_trigger_complementary(WorldObject* obj1, WorldObject* obj2) {
    PersonObj* person = (PersonObj*) obj1;
    MovableObj* movable = (MovableObj*) obj2;
    if (person->status != WorldObjectStatus::Tracked || person->name.empty()) return;
    if (movable->status == WorldObjectStatus::Hypothetical || movable->marker_id >= 200 || movable->marker_id <= 100) return;

    auto left_valid = person->latest_pose.certainty[7] > 0.3;
    auto right_valid = person->latest_pose.certainty[4] > 0.3;

    glm::vec3 aabb_offset = {0.0f, -0.35f, 0.1f};
    glm::vec3 aabb_extent = {0.1f, 0.30f, 0.25f};
    glm::vec3 aabb_origin = movable->center + aabb_offset;

    bool right_inside = glm::all(glm::lessThanEqual(glm::abs(person->latest_pose.joints[4] - aabb_origin), aabb_extent));
    bool left_inside = glm::all(glm::lessThanEqual(glm::abs(person->latest_pose.joints[7] - aabb_origin), aabb_extent));
    if (right_inside && right_valid) {
        //inform_about_false_belief_box(person->name, movable->name);
        //std::cout<<"*****HOLDING OBJECT "<< movable->name << "*******\n";
        //TODO: - search for objects that match the functional affordance of the holded object
        //      - add attention shift to that /those objects
        //      - inform in case there is no uncertainty and there is a false belief regarding that object
        inform_about_false_belief_object(person->name, movable->name);
    } else if (left_inside && left_valid) {
        //inform_about_false_belief_box(person->name, movable->name);
        std::cout<<"*****HOLDING OBJECT "<< movable->name << "*******\n";
        inform_about_false_belief_object(person->name, movable->name);
    }

}

static const float CUBE_TRACKING_DISTANCE = 0.25f;
static const int CUBE_TRACKING_TIMEOUT = 8;
static const int HEAD_TRACKING_FREQUENCY = 3;
static const int HEAD_TRACKING_SWITCH_FREQUENCY = 100;

bool Observing::tick(Robot& r) {
    sub->tick();
    if (current_sequence) {
        if (current_sequence->tick(r)) {
            current_sequence = nullptr;
        }
        return false;
    }

    //std::cout << "MOVABLE OBJECTS: \n";
    for (auto movable : r.world.iter_movables()) {
        /*
        if (movable->marker_id < 1000) { //Containers
            std::cout<< "Object " << movable->name  << " ";
            if(movable->type == PerceptionObjectType::Fruit)   std::cout<< "Fruit \n";
            else if(movable->type== PerceptionObjectType::Knife)   std::cout<< "Knife \n";
            else std::cout<< "wtf is the type \n";
            continue; 
        }*/
        epistemic.update_container(movable->name, movable->center);
    }

    std::vector<std::pair<std::string, glm::vec3>> potential_person_targets;
    std::vector<glm::vec3> potential_cube_targets;

    for (auto person : r.world.iter_persons()) {
        if (person->status != WorldObjectStatus::Tracked || person->name.empty()) continue;
        potential_person_targets.push_back({person->name, person->latest_pose.joints[0]});

        auto left_valid = person->latest_pose.certainty[7] > 0.3;
        auto right_valid = person->latest_pose.certainty[4] > 0.3;

        for (auto& movable : r.world.iter_movables()) {
            if (movable->status == WorldObjectStatus::Hypothetical) continue;

            //TODO: add same code for other objects
            if (movable->marker_id < 200) { /* Cubes and Objects*/
                auto distance_from_left_hand = glm::length(person->latest_pose.joints[7] - movable->center);
                auto distance_from_right_hand = glm::length(person->latest_pose.joints[4] - movable->center);
                if (left_valid && distance_from_left_hand < CUBE_TRACKING_DISTANCE) {
                    potential_cube_targets.push_back(movable->center);
                }
                if (right_valid && distance_from_right_hand < CUBE_TRACKING_DISTANCE) {
                    potential_cube_targets.push_back(movable->center);
                }
            }
            
        }
    }



    static int last_look_at_cube = -1;
    static std::string currently_track_target;
    static int person_switch_timer = 0;

    if (counter % HEAD_TRACKING_FREQUENCY == 0) {
        if (!potential_cube_targets.empty()) {
            last_look_at_cube = 0;
            r.look_at(potential_cube_targets[0]);
        } else if (!potential_person_targets.empty() && (last_look_at_cube < 0 || last_look_at_cube > CUBE_TRACKING_TIMEOUT)) {

            size_t chosen = 0;

            if (potential_person_targets.size() == 2) {
                chosen = potential_person_targets[0].first == currently_track_target ? 0 : 1;

                if (person_switch_timer > HEAD_TRACKING_SWITCH_FREQUENCY) {
                    chosen = chosen == 0 ? 1 : 0;
                    person_switch_timer = 0;
                }
            }
            currently_track_target = potential_person_targets[chosen].first;
            r.look_at(potential_person_targets[chosen].second);
            person_switch_timer++;
            last_look_at_cube = -1;
        }
        if (last_look_at_cube >= 0) {
            last_look_at_cube++;
        }
    }



    return false;
}
// Affordance mapping
std::unordered_map<std::string, std::string> affordance_map = {
    {"knife", "apple-105"},   // Knife can cut apple or bread
    {"apple", "knife"},           // Apple is cuttable by knife
    {"bottle", "glass"}, // Bottle can be filled with water or poured into a glass
    {"glass", "bottle"},          // Glass can receive liquid from a bottle
};

std::string get_matching_affordances(const std::string& obj) {
    auto it = affordance_map.find(obj);
    if (it != affordance_map.end()) {
        return it->second; // Return the respective affordances
    }
    return {}; // Return empty vector if no match
}


