#pragma once

#include "../state.h"

static const int WAITING_ANIMATE_FREQUENCY = 30*30;

class Waiting : public State {
public:
    void enter(Robot& r) override {
        counter = 0;
        done = false;
        r.toggle_breathing(true);
    }

    bool tick(Robot& r) override {
        if (++counter % WAITING_ANIMATE_FREQUENCY == 0) {
            auto animation = get_random_waiting_animation();
            r.play_animation(animation);
        }
        return done;
    }

    void confirm() override {
        done = true;
    }

    void leave(Robot& r) override {}

private:
    int counter = 0;
    bool done;
};
