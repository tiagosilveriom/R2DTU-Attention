#include <msgs/CameraMsgs.h>
#include <opencv2/opencv.hpp>
#include "StreamSensor.h"

StreamSensor::StreamSensor(Node *node, std::string channel) {
    intrinsics.width = 640;
    intrinsics.height= 480;
    intrinsics.fx = 556.8;
    intrinsics.fy = 555.9;
    intrinsics.cx = 309.4;
    intrinsics.cy = 230.6;
    subscriber = node->subscribe<ImageMsg>(channel);
}

Option<FrameSet> StreamSensor::poll_frames() {
    Option<std::shared_ptr<MessageHeader>> opt;

    //Empty out old images, in case the perception for some reason lags behind.
    while (true) {
        auto next = subscriber->next();
        if (!next.is_some()) break;
        opt = next;
    }

    if (!opt.is_some()) return Option<FrameSet>();

    auto msg = Messages::content<ImageMsg>(opt->get());

    ASSERTF(msg->width == intrinsics.width && msg->height == intrinsics.height, "Received image of size %ux%u, but expected %ux%u\n",
            msg->width, msg->height, intrinsics.width, intrinsics.height);

    auto[data_ptr, data_len] = Messages::access_blob(&msg->payload);

    auto img = cv::imdecode(cv::Mat(1, data_len, CV_8UC1, data_ptr), cv::IMREAD_ANYCOLOR);

    cv::cvtColor(img, img, CV_BGR2RGB);

    return Option<FrameSet>({img, Option<cv::Mat>(), Option<cv::Mat>(), intrinsics});
}
