#pragma once

#include "robot.h"

class State {
public:
    virtual void enter(Robot& r) = 0;
    virtual bool tick(Robot& r) = 0;
    virtual void leave(Robot& r) = 0;
    virtual void confirm() {};
};

class Sequence {
public:
    virtual bool tick(Robot& r) = 0;
    virtual void confirm() {};
};
