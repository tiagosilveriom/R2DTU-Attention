#include <core/Node.h>
#include <core/Module.h>
#include <common/SubscriptionHandler.h>
#include <msgs/CameraMsgs.h>
#include <opencv2/opencv.hpp>

class MockBotPlayer {
public:

    explicit MockBotPlayer(Node* node) : node_(node) {
        using namespace std::placeholders;

        forward_pub = node->advertise<ImageMsg>("vis_forward");
        down_pub = node->advertise<ImageMsg>("vis_down");
        skeleton_pub = node->advertise<SkeletonMsg>("skeleton");

        auto path = *node->access_kv().get("mockbot-dir") + "/";

        forward_reader.open(path + "forward.avi");
        down_reader.open(path + "down.avi");
        skeleton_reader = fopen((path + "skeleton.bin").c_str(), "rb");
        fread(&next_skeleton, sizeof(uint32_t), 1, skeleton_reader);
    }

    ~MockBotPlayer() {}

    void publish_frame(cv::VideoCapture& cap, Publisher* pub) {

        cv::Mat frame;
        if (!cap.read(frame)) {
            done = true;
            return;
        }

        std::vector<uint8_t> compressed_frame;
        std::vector<int32_t> compression_flags;
        compression_flags.push_back(cv::IMWRITE_JPEG_QUALITY);
        compression_flags.push_back(70);

        cv::imencode(".jpg", frame, compressed_frame, compression_flags);

        MessageBuilder<ImageMsg> msg;

        msg->width = frame.cols;
        msg->height = frame.rows;
        msg.write_blob(&msg->payload, &compressed_frame[0], compressed_frame.size());

        ++frame_count;

        pub->publish(msg);
    }

    void publish_skeletons() {
        MessageBuilder<SkeletonMsg> msg;
        while (frame_count >= next_skeleton) {
            if (fread(&msg->user_id, sizeof(SkeletonMsg), 1, skeleton_reader)) {
                skeleton_pub->publish(msg);
                if (!fread(&next_skeleton, sizeof(uint32_t), 1, skeleton_reader)) {
                    goto fail;
                }
            } else {
                goto fail;
            }
        }
        return;
        fail:
        next_skeleton = INT32_MAX;
    }

    void tick() {
        if (!done) {
            publish_frame(forward_reader, forward_pub);
            publish_frame(down_reader, down_pub);
            publish_skeletons();
        }
    }

private:
    Node* node_;
    Publisher* forward_pub;
    Publisher* down_pub;
    Publisher* skeleton_pub;

    cv::VideoCapture forward_reader;
    cv::VideoCapture down_reader;
    FILE* skeleton_reader;

    uint32_t frame_count = 0;
    uint32_t next_skeleton = 0;


    bool done = false;
};

extern "C" {

ModuleState* initialize(Node* node) {
    return new MockBotPlayer(node);
}

void tick(Node* node, MockBotPlayer* bot) {
    bot->tick();
}

void shutdown(Node* node, MockBotPlayer* bot) {
    delete bot;
}

void unload(Node* node, MockBotPlayer* bot) {

}

void reload(Node* node, MockBotPlayer* bot) {

}

} //extern "C"

