//
// Created by dissing on 8/8/19.
//

#include "Realsense.h"

struct Context {
    rs2::context rs_ctx;
};

static Context ctx;

bool RealSense::initialize(const char* wanted_serial, bool sharp) {

    serial = wanted_serial;
    while (true) {
        printf("Trying opening %s\n", wanted_serial);
        try {
            for (auto&& dev : ctx.rs_ctx.query_devices()) {
                auto dev_serial = dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
                if (strcmp(dev_serial, wanted_serial) == 0) {
                    pipe = rs2::pipeline(ctx.rs_ctx);
                    cfg.enable_device(dev_serial);
                    cfg.enable_stream(RS2_STREAM_DEPTH);
                    cfg.enable_stream(RS2_STREAM_COLOR, 1280, 720, RS2_FORMAT_RGB8, 30);
                    auto profile = pipe.start(cfg);

                    rs_intrinsics = profile.get_stream(RS2_STREAM_COLOR).as<rs2::video_stream_profile>().get_intrinsics(); // Calibration data
                    init_flag = true;
                    intrinsics.fx = rs_intrinsics.fx;
                    intrinsics.fy = rs_intrinsics.fy;
                    intrinsics.cx = rs_intrinsics.ppx;
                    intrinsics.cy = rs_intrinsics.ppy;
                    intrinsics.width = rs_intrinsics.width;
                    intrinsics.height = rs_intrinsics.height;
                    return true;
                }
            }
            printf("ERROR: Failed to find Realsense camera with S/N: %s\n", wanted_serial);
            return false;
        } catch (rs2::invalid_value_error e) {
            printf("Value error\n");
        } catch (rs2::backend_error e) {
            printf("Backend error\n");
        }
    }
    colorizer = rs2::colorizer();
}

Option<FrameSet> RealSense::poll_frames() {
    if (!init_flag) {
        return Option<FrameSet>();
    }

    rs2::frameset data;
    if (!pipe.poll_for_frames(&data)) {
        return Option<FrameSet>();
    }

    data = align_to_color.process(data);


    rs2::depth_frame depth = data.get_depth_frame();
    depth = hole_filter.process(depth);
    rs2::frame color = data.get_color_frame();
    rs2::frame depth_vis = colorizer.process(depth);

    // Query frame size (width and height)
    const int cw = color.as<rs2::video_frame>().get_width();
    const int ch = color.as<rs2::video_frame>().get_height();
    const int dw = depth.as<rs2::video_frame>().get_width();
    const int dh = depth.as<rs2::video_frame>().get_height();

    // Create OpenCV matrix of size (w,h) from the colorized depth data
    cv::Mat depth_img(cv::Size(dw, dh), CV_16UC1);
    cv::Mat color_img(cv::Size(cw, ch), CV_8UC3);
    cv::Mat depth_vis_img(cv::Size(dw, dh), CV_8UC3);
    memcpy(depth_img.data, (void *) depth.get_data(), dw * dh * sizeof(uint16_t));
    memcpy(color_img.data, (void *) color.get_data(), cw * ch * sizeof(uint8_t) * 3);
    memcpy(depth_vis_img.data, (void *) depth_vis.get_data(), dw * dh * sizeof(uint8_t) * 3);

    FrameSet set = {color_img, depth_img, depth_vis_img, intrinsics};

    return Option(set);
}
