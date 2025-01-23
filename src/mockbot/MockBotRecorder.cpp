#include <sys/stat.h>

#include <core/Node.h>
#include <core/Module.h>
#include <common/SubscriptionHandler.h>
#include <msgs/CameraMsgs.h>
#include <opencv2/opencv.hpp>

class MockBotRecorder {
public:

    void handle_image(void* usr_ptr, ImageMsg const* msg) {

        auto[data_ptr, data_len] = Messages::access_blob(&msg->payload);
        auto img = cv::imdecode(cv::Mat(1, data_len, CV_8UC1, data_ptr), cv::IMREAD_ANYCOLOR);

        auto writer = (cv::VideoWriter*) usr_ptr;
        writer->write(img);
        frame_count++;
    }

    void handle_skeleton(void* usr_ptr, SkeletonMsg const* msg) {
        fwrite(&frame_count, sizeof(uint32_t), 1, this->skeleton_writer);
        fwrite(msg, sizeof(SkeletonMsg), 1, this->skeleton_writer);
    }

    explicit MockBotRecorder(Node* node) : node_(node), sub(node) {
        using namespace std::placeholders;

        auto path = *node->access_kv().get("mockbot-dir") + "/";

        mkdir(path.c_str(), 0777);

        sub.subscribe<ImageMsg>("vis_forward", std::bind(&MockBotRecorder::handle_image, this, _1, _2), &forward_writer);
        sub.subscribe<ImageMsg>("vis_down", std::bind(&MockBotRecorder::handle_image, this, _1, _2), &down_writer);
        sub.subscribe<SkeletonMsg>("skeleton", std::bind(&MockBotRecorder::handle_skeleton, this, _1, _2), nullptr);
        forward_writer.open(path + "forward.avi", cv::VideoWriter::fourcc('M','J','P','G'), 30, cv::Size(640, 480));
        down_writer.open(path + "down.avi", cv::VideoWriter::fourcc('M','J','P','G'), 30, cv::Size(640, 480));
        skeleton_writer = fopen((path + "skeleton.bin").c_str(), "wb");
    }

    ~MockBotRecorder() {
        forward_writer.release();
        down_writer.release();
    }

    void tick() {
        sub.tick();
    }

private:
    Node* node_;
    SubscriptionHandler sub;
    cv::VideoWriter forward_writer;
    cv::VideoWriter down_writer;
    FILE* skeleton_writer;
    uint32_t frame_count;
};

extern "C" {

ModuleState* initialize(Node* node) {
    return new MockBotRecorder(node);
}

void tick(Node* node, MockBotRecorder* bot) {
    bot->tick();
}

void shutdown(Node* node, MockBotRecorder* bot) {
    delete bot;
}

void unload(Node* node, MockBotRecorder* bot) {

}

void reload(Node* node, MockBotRecorder* bot) {

}

} //extern "C"

