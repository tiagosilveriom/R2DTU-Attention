#pragma once

#include <common/Vectors.h>
#include <common/RobotDefs.h>

#include "MsgsStart.h"

struct JointSensorMsg {
    float angles[NUMBER_OF_JOINTS];
};

struct HealthTelemetryMsg {
    int8_t joint_temps[NUMBER_OF_JOINTS];
    float joint_amps[NUMBER_OF_JOINTS];
    int8_t battery;
    int8_t cpu_temp;
    uint8_t cpu_usage;
    uint32_t ram;
};

struct TouchEventMsg {
    enum Type : uint8_t {
        HEAD_FRONT      = 0b00000001,
        HEAD_MID        = 0b00000010,
        HEAD_REAR       = 0b00000100,
        HEAD_ANY        = 0b00000111,
        HAND_L          = 0b00001000,
        HAND_R          = 0b00010000,
        HAND_ANY        = 0b00011000,
        BUMPER_LEFT     = 0b00100000,
        BUMPER_RIGHT    = 0b01000000,
        BUMPER_BACK     = 0b10000000,
        BUMPER_ANY      = 0b11100000,
        ANY_TOUCH       = 0b11111111
    };
    Type type;
    bool engaged;
};

#include "MsgsEnd.h"