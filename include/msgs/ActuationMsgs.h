#pragma once

#include <common/Vectors.h>
#include <common/Color.h>
#include <common/RobotDefs.h>

#include "MsgsStart.h"

struct ActuationRequest {
    uint32_t request_id;
    uint16_t requester_id;
};


struct JointActuationMsg {
    ActuationRequest request;
    float speed;
    int8_t ids[NUMBER_OF_JOINTS];
    float angles[NUMBER_OF_JOINTS];
    bool relative_angles;
};

struct MoveActuationMsg {
    ActuationRequest request;
    float forward;
    float left;
    float theta;
};

struct AnimationActuationMsg {
    ActuationRequest request;
    MsgText name;
};

struct LedActuationMsg {
    ActuationRequest request;
    uint8_t group;
    Color color;
};

struct SayActuationMsg {
    ActuationRequest request;
    MsgText sentence;
    bool failed_to_understand;
};

struct ShowTabletActuationMsg {
    ActuationRequest request;
    MsgText filename;
};

struct ActuationCompletionMsg {
    ActuationRequest response;
};

struct BuiltinFlagsMsg {
    enum class Flag {
        FaceTracking,
    };
    Flag flag;
    bool enable;
};

#include "MsgsEnd.h"