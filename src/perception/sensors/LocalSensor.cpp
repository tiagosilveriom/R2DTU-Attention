#include <msgs/CameraMsgs.h>
#include <opencv2/opencv.hpp>
#include "LocalSensor.h"

LocalSensor::LocalSensor(uint32_t id) {
    intrinsics.width = 640;
    intrinsics.height= 480;
    intrinsics.fx = 556.8;
    intrinsics.fy = 555.9;
    intrinsics.cx = 309.4;
    intrinsics.cy = 230.6;
    capture.open(id);
    if (!capture.isOpened()) {
        printf("Failed to find AUX camera\n");
    }
}

Option<FrameSet> LocalSensor::poll_frames() {
    if (!capture.isOpened()) {
        return Option<FrameSet>();
    }
    capture >> frame;

    cv::cvtColor(frame, frame, CV_BGR2RGB);

    return Option<FrameSet>({frame, Option<cv::Mat>(), Option<cv::Mat>(), intrinsics});
}
