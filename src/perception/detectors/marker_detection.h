#pragma once

#include "../Perception.h"
#include <opencv2/core/core.hpp>
#include "perception/percept.h"
#include <glm/mat4x4.hpp>

namespace MarkerDetection {

    void initialize();

    void detect(FrameSet frames, std::vector<MarkerPercept> &percepts);

    glm::mat4x4 measure(cv::Mat img, uint32_t fiducial, float fx, float fy, float cx, float cy);

    std::vector<MarkerPercept> drawings();

    void shutdown();
}