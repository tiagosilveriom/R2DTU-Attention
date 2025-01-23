
#include <core/Module.h>
#include <asr/ASR.h>
#include "state.h"

#include "states/Calibration.h"
#include "states/Greet.h"
#include "states/Waiting.h"
#include "states/Observing.h"

class FBT1 {
public:
    FBT1(Node* node) : robot(node) {
        sub = std::make_unique<SubscriptionHandler>(node);
        sub->subscribe<StateCmdMsg>("state_cmd", std::bind(&FBT1::state_cmd, this, std::placeholders::_1, std::placeholders::_2), nullptr);
        sub->subscribe<TerminalCmdMsg>("terminal_cmd", std::bind(&FBT1::terminal_cmd, this, std::placeholders::_1, std::placeholders::_2), nullptr);
        text_drawing_pub = node->advertise<TextDrawingMsg>("operator_view_perception_1_text_drawing");
        state = nullptr;
        states.push_back(new Calibration());
        states.push_back(new Waiting());
        states.push_back(new Greet(node));
        states.push_back(new Observing(node));
        goto_state(0);
    }

    void tick() {
        sub->tick();
        robot.tick();
        if (state->tick(robot)) {
            goto_state(current_state + 1);
        }
        if (asr_stop_timer > 0) {
            asr_stop_timer--;
        } else if (asr_stop_timer == 0) {
            auto text = robot.finish_recording();
            robot.send_to_terminal(text);
            asr_stop_timer = -1;
        }

        for (auto& person : robot.world.iter_persons()) {
            if (person->status == WorldObjectStatus::Tracked) {
                MessageBuilder<TextDrawingMsg> msg;
                msg.write_string(&msg->text, person->name);
                msg->x = person->latest_pose.raw_joints[0].x * 640 / 1920;
                msg->y = person->latest_pose.raw_joints[0].y * 480 / 1080 + 100;

                text_drawing_pub->publish(msg);
            }
        }
    }

    void goto_state(uint32_t s) {
        if (s < 0 || s >= states.size()) return;
        if (state) {
            state->leave(robot);
        }
        state = states[s];
        current_state = s;
        state->enter(robot);
    }

    void terminal_cmd(void* usr, const TerminalCmdMsg* msg) {
        auto cmd_type = Messages::read_string(&msg->cmd_type);
        auto param = Messages::read_string(&msg->param);
        if (cmd_type == "asr" && param == "check") {
            auto s = cloud_listen();
        }
    }

    void state_cmd(void* usr, const StateCmdMsg* msg) {
        if (msg->type == StateCmdMsg::Next) {
            goto_state(current_state + 1);
        } else if (msg->type == StateCmdMsg::Back) {
            goto_state(current_state - 1);
        } else if (msg->type == StateCmdMsg::Confirm) {
            state->confirm();
        }
    }

private:
    Robot robot;
    Publisher* text_drawing_pub;
    std::vector<State*> states;
    State* state;
    int current_state;
    std::unique_ptr<SubscriptionHandler> sub;
    int asr_stop_timer = -1;
};

extern "C" {

ModuleState* initialize(Node* node) {
    return new FBT1(node);
}

void tick(Node* node, FBT1* fbt) {
    fbt->tick();
}

void shutdown(Node* node, FBT1* fbt) {
    delete fbt;
}

void unload(Node* node, FBT1* fbt) {

}

void reload(Node* node, FBT1* fbt) {

}

} //extern "C"
