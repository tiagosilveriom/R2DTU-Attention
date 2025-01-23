#pragma once

#include "../state.h"
#include <unordered_set>

class Greet : public State {
public:

    Greet(Node* node) : sub(std::make_unique<SubscriptionHandler>(node)) {
        rect_scan_pub = node->advertise<RectScanCmdMsg>("rect_scan_cmd");
        sub->subscribe<TerminalCmdMsg>("terminal_cmd", std::bind(&Greet::terminal_cmd, this, std::placeholders::_1, std::placeholders::_2), nullptr);
    }

    void terminal_cmd(void* usr, const TerminalCmdMsg* msg);

    void confirm() override;
    void enter(Robot& r) override;
    void leave(Robot& r) override;
    bool tick(Robot& r) override;

private:
    int counter = 0;
    int confirm_counter = 0;
    bool confirm_flag = false;
    bool head_tracking = false;
    bool greet_in_progress;

    std::string newly_learned = "";

    uint32_t current_target;

    std::unordered_set<uint32_t> already_greeted_ids;
    Robot* robot;

    Publisher* rect_scan_pub;
    std::unique_ptr<SubscriptionHandler> sub;
    bool recording = false;
};