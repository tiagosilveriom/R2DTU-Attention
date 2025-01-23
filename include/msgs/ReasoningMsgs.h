#pragma once

#include <common/Color.h>

#include "MsgsStart.h"

struct EpistemicStateViewMsg {
    const static size_t MAX_CONTAINERS = 4;
    const static size_t MAX_MOVABLES = 7;
    char containers[MAX_CONTAINERS];
    char movables_location[MAX_MOVABLES];
    char movables_id[MAX_MOVABLES];
    MsgText name;
};

#include "MsgsEnd.h"
