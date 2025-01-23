#pragma once

#include <common/Option.h>
#include <opencv2/core.hpp>
#include <glm/vec3.hpp>

struct Intrinsics {
    uint32_t width;
    uint32_t height;
    float fx;
    float fy;
    float cx;
    float cy;
};

struct FrameSet {
    Option<cv::Mat> colour;
    Option<cv::Mat> depth;
    Option<cv::Mat> depth_viz;
    Intrinsics intrinsics;
};

class Sensor {
public:
    virtual Option<FrameSet> poll_frames() = 0;
    virtual const Intrinsics& get_intrinsics() = 0;
};

glm::vec3 deproject_pixel(const Intrinsics& intr, glm::vec3 pixel);
