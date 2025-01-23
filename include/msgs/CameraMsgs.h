#pragma once

#include <common/Vectors.h>
#include "MsgsStart.h"

struct ImageMsg {
    uint32_t timestamp;
    uint16_t width;
    uint16_t height;
    uint8_t components;
    MsgBlob payload;
};

struct DepthImageMsg {
    uint16_t width;
    uint16_t height;
    MsgBlob payload;
};

const uint32_t NUMBER_OF_SKELETON_BONES = 15;
enum Bone {
    HEAD = 0,
    NECK = 1,

    L_SHOULDER = 2,
    R_SHOULDER = 3,
    L_ELBOW = 4,
    R_ELBOW = 5,
    L_HAND = 6,
    R_HAND = 7,

    TORSO = 8,

    L_HIP = 9,
    R_HIP = 10,
    L_KNEE = 11,
    R_KNEE = 12,
    L_FOOT = 13,
    R_FOOT = 14
};

struct SkeletonMsg {
    uint8_t user_id;
    Vec3 joints[NUMBER_OF_SKELETON_BONES];
    float certainty[NUMBER_OF_SKELETON_BONES];
};

#include "MsgsEnd.h"
