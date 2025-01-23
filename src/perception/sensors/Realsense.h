#pragma once

#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>
#include <librealsense2/rsutil.h>

#include "../Perception.h"

class RealSense : public Sensor {
public:
    RealSense() : align_to_color(RS2_STREAM_COLOR), init_flag(false) {};

    bool initialize(const char* serial, bool sharp);

    Option<FrameSet> poll_frames() override;

    const Intrinsics& get_intrinsics() { return intrinsics; }

private:
    rs2::pipeline pipe;
    rs2::config cfg;
    rs2::hole_filling_filter hole_filter;

    rs2::align align_to_color;
    rs2_intrinsics rs_intrinsics;
    Intrinsics intrinsics;
    const char* serial;
    rs2::colorizer colorizer;

    bool init_flag;
};

