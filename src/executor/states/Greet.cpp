#include "Greet.h"
#include <l10n/FBT.h>

static const int HEAD_TRACKING_FREQUENCY = 3;
static const int HEAD_TRACKING_SWITCH_FREQUENCY = 50;

void Greet::terminal_cmd(void* usr, const TerminalCmdMsg* msg) {
    auto cmd = Messages::read_string(&msg->cmd_type);
    auto param = Messages::read_string(&msg->param);
    if (cmd == "sr") {
        if (!recording) {
            recording = true;
            robot->start_recording();
        } else {
            auto text = robot->finish_recording();
            robot->log_info("Robot", "\"" + text + "\"");
            std::string name;
            for (auto i = 0u; i < text.size(); ++i) {
                if (text[i] != ' ' && text[i] != '?' && text[i] != '"') {
                    name.push_back( i == 0 ? toupper(text[i]) : text[i]);
                }
            }
            robot->send_to_terminal("learn " + name);
            recording = false;
        }
    } else if (cmd == "learn") {
        newly_learned = param;
    }
}

void Greet::confirm() {
    confirm_flag = true;
}

void Greet::enter(Robot& r) {
    r.toggle_breathing(false);
    r.move_to(glm::vec3 {0.00, 0.0, -0.3});
    greet_in_progress = false;
    current_target = 0;
    counter = 0;
    confirm_counter = 0;
    confirm_flag = false;
    robot = &r;
}
void Greet::leave(Robot& r) {
    r.turn(0);
    r.move_to(glm::vec3 {0.00, 0.0, -0.6});
}

bool Greet::tick(Robot& r) {

    glm::vec3 target_head;
    bool target_tracked = false;

    for (auto person : r.world.iter_persons()) {
        bool tracked = person->status == WorldObjectStatus::Tracked;
        auto search = already_greeted_ids.find(person->id);
        if (confirm_flag && !greet_in_progress && tracked && person->name.empty() && search == already_greeted_ids.end()) {
            greet_in_progress = true;
            current_target = person->id;
            counter = 0;
            confirm_counter = 0;
            confirm_flag = false;
            head_tracking = true;
            newly_learned = "";
        }
        if (person->id == current_target) {
            target_head = person->latest_pose.joints[0];
            target_tracked = tracked;
        }
    }

    static int person_switch_timer = 0;

    if (!greet_in_progress) {
        std::vector<uint32_t > other_targets;
        for (auto person : r.world.iter_persons()) {
            if (person->status == WorldObjectStatus::Tracked && person->id != current_target) {
                other_targets.push_back(person->id);
            }
        }
        if (person_switch_timer > HEAD_TRACKING_SWITCH_FREQUENCY * 3 && !other_targets.empty()) {
            current_target = other_targets[0];
            person_switch_timer = 0;
            return false;
        }
        person_switch_timer++;
    }


    sub->tick();
    if (greet_in_progress) {
        if (confirm_counter == 0) {
            if (counter == 0) {
                head_tracking = false;
                r.set_head_angles(0.0, 0.0, 300);
            }
            if (counter == 15 && target_tracked) {
                auto dir = target_head - r.get_current_pos();
                auto theta = -atan2(dir.x, dir.z);
                r.turn(theta);
            }
            if (counter == 120) {
                r.start_learn(current_target);
                r.handshake_up();
                r.say(L10N_HELLO);
            }
            if (counter == 150) {
                head_tracking = true;
            }
            if (confirm_flag) {
                r.say(L10N_MY_NAME);
                r.handshake_reciprocate();
                confirm_flag = false;
                confirm_counter++;
            }
        } else if (confirm_counter == 1) {

            if (confirm_flag) {
                head_tracking = false;
                r.handshake_down();
                r.say(L10N_YOUR_NAME);
                confirm_flag = false;
                confirm_counter++;
                counter = 0;
            }

            if (counter == 30) {
                head_tracking = true;
            }
        } else if (confirm_counter == 2) {
            if (counter > 30) {
                bool person_learned = false;
                for (auto person : r.world.iter_persons()) {
                    if (!newly_learned.empty() && person->name == newly_learned) {
                        person_learned = true;
                    }
                }
                if (person_learned || confirm_flag) {
                    char buffer[128];
                    sprintf(buffer, L10N_NICE_TO_MEET, newly_learned.c_str());
                    already_greeted_ids.insert(current_target);
                    r.say(buffer);
                    newly_learned = "";
                    counter = 0;
                    confirm_counter = 0;
                    confirm_flag = false;
                    greet_in_progress = false;
                }
            }
        }
    }

    if (head_tracking && counter % HEAD_TRACKING_FREQUENCY == 0 && target_tracked) {
        r.look_at(target_head);
    }

    counter++;
    return false;
}


