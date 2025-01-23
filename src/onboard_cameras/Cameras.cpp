#include <opencv2/opencv.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wplacement-new="
#include <alcommon/albroker.h>
#include <alvision/alimage.h>
#include <alproxies/alvideodeviceproxy.h>
#include <alproxies/almemoryproxy.h>
#pragma GCC diagnostic pop

#include <msgs/CameraMsgs.h>
#include <common/Timer.h>
#include <core/Node.h>
#include <core/Module.h>

#include "OV5640.h"

struct CameraInfo {
    OV5640::Device* device;
    cv::Mat buffer;
    std::atomic<bool> running;
    pthread_t worker;
    Publisher* pub;
};

struct Context {

    boost::shared_ptr<AL::ALBroker> broker;
    AL::ALVideoDeviceProxy* al_video;
    AL::ALMemoryProxy* al_mem;

    CameraInfo forward_cam;
    CameraInfo down_cam;

    uint32_t num_cam_enabled;

    uint8_t* buffer;

    std::atomic<bool> running;
};

void configure_camera(CameraInfo* info) {
    OV5640::set_setting(info->device, OV5640::Setting::Brightness, 0);
    OV5640::set_setting(info->device, OV5640::Setting::Contrast, 32);
    OV5640::set_setting(info->device, OV5640::Setting::Saturation, 64);
    OV5640::set_setting(info->device, OV5640::Setting::WhiteBalanceAutomatic, 0);
    OV5640::set_setting(info->device, OV5640::Setting::GainAutomatic, 0);
    OV5640::set_setting(info->device, OV5640::Setting::Gain, 16);
    OV5640::set_setting(info->device, OV5640::Setting::ExposureAutomatic, 0);
    OV5640::set_setting(info->device, OV5640::Setting::Exposure, 1024);
}

void publish_img(cv::Mat img, Publisher* pub) {

    //auto start = Timer::timestamp();
    auto width = (uint16_t) img.cols;
    auto height = (uint16_t) img.rows;

    std::vector<uint8_t> compressed_frame;

    std::vector<int32_t> compression_flags;
    compression_flags.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_flags.push_back(45);

    cv::imencode(".jpg", img, compressed_frame, compression_flags);


    MessageBuilder<ImageMsg> msg;

    msg->width = width;
    msg->height = height;
    msg.write_blob(&msg->payload, &compressed_frame[0], compressed_frame.size());

    //auto end = Timer::timestamp();
    //auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    pub->publish(msg);
}

extern "C" {

void* camera_worker_thread(void* raw) {
    auto info = (CameraInfo*) raw;
    while (info->running) {
        OV5640::Frame temp = OV5640::fetch_frame(info->device);
        cv::Mat frame_yuyv(480, 640, CV_8UC2, temp.data);
        cv::cvtColor(frame_yuyv, info->buffer, cv::COLOR_YUV2BGR_YUYV);
        publish_img(info->buffer, info->pub);
    }
    return nullptr;
}

void init_cam_info(CameraInfo* info, const char* path, int id) {
    info->running = true;
    info->device = OV5640::initialize(path, id);
    configure_camera(info);
    pthread_create(&info->worker, nullptr, camera_worker_thread, info);
}

ModuleState* initialize(Node* node) {

    auto mem = malloc(sizeof(Context));
    auto ctx = new(mem) Context();

    ctx->num_cam_enabled = 1; //node->kv_get("visual_camera", 0);

    ctx->broker = AL::ALBroker::createBroker("LocalBroker", "0.0.0.0", 54000, "127.0.0.1", 9559);

    //Unsubscribe all NaoQi modules, such that we can claim the raw hardware interfaces
    ctx->al_video = new AL::ALVideoDeviceProxy();
    std::vector<std::string> subs = ctx->al_video->getSubscribers();
    for (auto &sub : subs) {
        ctx->al_video->unsubscribe(sub);
    }

    // Determine robot version since there are slight differences in the camera hardware
    auto al_mem = AL::ALMemoryProxy();
    std::string version = al_mem.getData("RobotConfig/Body/BaseVersion");
    printf("Robot version: %s\n", version.c_str());


    int forward_cam_path_id;
    int forward_cam_device_id;
    int down_cam_path_id;
    int down_cam_device_id;

    if (version == "1.8") {
	forward_cam_path_id = 1;
	forward_cam_device_id = 0;
	down_cam_path_id = 2;
	down_cam_device_id = 0;
    } else if (version == "1.8A") {
	forward_cam_path_id = 0;
	forward_cam_device_id = 0;
	down_cam_path_id = 1;
	down_cam_device_id = 1;
    } else {
	printf("Unsupported robot version!\n");
	printf("Please add support in onboard_cameras/Cameras.cpp!\n");
        std::exit(-1);
    }

    if (ctx->num_cam_enabled > 0){
	auto forward_path = "/dev/video" + std::to_string(forward_cam_path_id);
        init_cam_info(&ctx->forward_cam, forward_path.c_str(), forward_cam_device_id);
        ctx->forward_cam.pub = node->advertise<ImageMsg>("vis_forward");
    }

    if (ctx->num_cam_enabled > 1) {
	auto down_path = "/dev/video" + std::to_string(down_cam_path_id);
        init_cam_info(&ctx->down_cam, down_path.c_str(), down_cam_device_id);
        ctx->down_cam.pub = node->advertise<ImageMsg>("vis_down");
    }


    return ctx;
}

void tick(Node *node, Context* ctx) {

}

void destroy_cam_info(CameraInfo* info) {
    info->running = false;
    pthread_join(info->worker, nullptr);
    OV5640::close(info->device);
}

void shutdown(Node *node, Context* ctx) {
    if (ctx->num_cam_enabled > 0) destroy_cam_info(&ctx->forward_cam);
    if (ctx->num_cam_enabled > 1) destroy_cam_info(&ctx->down_cam);
    delete ctx->al_video;
    delete ctx->al_mem;
}

void unload(Node* node, Context* ctx) {

}

void reload(Node* node, Context* ctx) {

}

} //extern "C"
