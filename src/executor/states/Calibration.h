#pragma once
#include "../state.h"

class Calibration : public State {
public:

    void enter(Robot& r) override ;

    bool tick(Robot& r) override;

    void leave(Robot& r) override {}

private:
    int counter;
    int move_stage = 0;
};