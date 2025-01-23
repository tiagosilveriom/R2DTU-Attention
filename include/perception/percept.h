#pragma once

#include <stdint.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <string>
#include <opencv2/core/types.hpp>
#include <iostream>


enum class PerceptionObjectType {
    Bottle,
    Knife,
    Fruit,
    Cup,
    Marker,
    Unknown
};

struct MarkerPercept {
    glm::mat4x4 transform;
    uint32_t id;
    glm::vec3 pos;
    glm::vec2 raw_pos;
};

const size_t POSE_JOINTS = 25;

struct SkeletonPercept {
    uint32_t id;
    glm::vec3 joints[POSE_JOINTS];
    glm::vec2 raw_joints[POSE_JOINTS];
    float certainty[POSE_JOINTS];
    glm::vec2 box_size;
};

struct PersonPercept {
    uint32_t id;
    const char* name;
    SkeletonPercept skeleton;
};

struct ObjectPercept {
    uint32_t id;
    std::string name;
    PerceptionObjectType type;
    // For now  I will use bounding box, but maybe center point will be more useful and a change will be needed
    cv::Rect2f bounding_box; 
    glm::vec3 pos; // Bounding box center world space
    glm::vec2 raw_pos; // Bounding box center camera frame

    //std::vector<std::string> affordances;
};

