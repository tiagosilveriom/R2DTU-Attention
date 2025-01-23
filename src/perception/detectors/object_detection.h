#pragma once

#include "../Perception.h"
#include "perception/percept.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <darknet/include/darknet.h>

namespace ObjectDetection {

    void initialize();

    void detect(FrameSet frames, std::vector<ObjectPercept> &percepts);

    void shutdown();

    image mat_to_image(const cv::Mat& mat);

    PerceptionObjectType get_object_type(std::string object_name);

    std::vector<std::string> get_object_affordances(std::string object_name);
    
    uint32_t get_object_id(std::string object_name);
}

