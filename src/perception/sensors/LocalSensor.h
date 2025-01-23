#pragma once

#include <core/Node.h>
#include "../Perception.h"
#include <opencv2/core.hpp>

class LocalSensor : public Sensor {
public:
    LocalSensor(uint32_t id);

    Option<FrameSet> poll_frames() override;

    const Intrinsics& get_intrinsics() { return intrinsics; }
private:
    Intrinsics intrinsics;
    cv::VideoCapture capture;
    cv::Mat frame;
};
