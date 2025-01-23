#pragma once

#include "ActuationMsgs.h"

#include "MsgsStart.h"

struct CmdCompletionMessage {
    ActuationRequest response;
};

struct SkillCommandMsg {
    uint32_t counter_id;
    uint16_t num_params;
    uint16_t requester;
    MsgBlob payload;
};

struct SkillResponseMsg {
    uint32_t counter_id;
    uint16_t requester;
};

struct SkillCancelMsg {
    uint32_t counter_id;
    uint16_t requester;
};

#include "MsgsEnd.h"
