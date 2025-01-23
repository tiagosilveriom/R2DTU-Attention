#pragma once

#include <perception/percept.h>

#include "MsgsStart.h"

struct PerceptsMsg {
    MarkerPercept* markers;
    size_t num_markers;
    PersonPercept* persons;
    size_t num_persons;
    int source_index;
    ObjectPercept* objects;
    size_t num_objects;
};

struct RectScanCmdMsg {
    glm::vec3 center;
    float width, height;
    uint32_t source_id;
    bool enable;
};

struct RectScanRespMsg {
    glm::vec3 mean_color;
};

#include "MsgsEnd.h"
