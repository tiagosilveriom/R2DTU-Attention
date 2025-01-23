#pragma once
#include "MsgsStart.h"

struct KeyValueMsg {
    uint32_t value;
    MsgText key;
    uint8_t sender;
};

struct LifecycleMsg {
    enum Type {
        START,
        STOP,
    };

    Type type;
    uint8_t sender;


};

struct SubsystemActivationMsg {
    bool enable;
};

#include "MsgsEnd.h"