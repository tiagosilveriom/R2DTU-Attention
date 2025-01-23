#include "skeleton_detection.h"

#define OPENPOSE_FLAGS_DISABLE_POSE

#include <openpose/flags.hpp>
#include <openpose/headers.hpp>

namespace SkeletonDetection {

    op::Wrapper* op;

    std::vector<SkeletonPercept> detect(FrameSet frames) {

        std::vector<SkeletonPercept> percepts;

        auto result = op->emplaceAndPop(*frames.colour);
        if (result && !result->empty()) {
            auto poses = result->at(0);
            auto num_people = poses->poseIds.getVolume();
            auto stride = 25 * 3;
            for (size_t i = 0; i < num_people; ++i) {
                SkeletonPercept p;
                p.id = poses->poseIds.at(i);
                for (size_t j = 0; j < 25; ++j) {
                    glm::vec2 raw;
                    glm::vec3 v;
                    raw.x = v.x = poses->poseKeypoints[i * stride + j * 3];
                    raw.y = v.y = poses->poseKeypoints[i * stride + j * 3 + 1];
                    v.z = (float) (frames.depth->at<uint16_t>(v.y, v.x) * 0.001);
                    p.joints[j] = deproject_pixel(frames.intrinsics, v);
                    p.raw_joints[j] = raw;
                    p.certainty[j] =  poses->poseKeypoints[i * stride + j * 3 + 2];
                }
                if (p.joints[1].z < 2.5) {
                    percepts.push_back(p);
                }
            }
        }
        return percepts;
    }

    void initialize() {
        op = new op::Wrapper { op::ThreadManagerMode::Asynchronous};
        op->configure(op::WrapperStructFace{false});
        op->configure(op::WrapperStructHand{false});
        op::WrapperStructPose pose_config{};
        pose_config.netInputSize = op::flagsToPoint("-640x368", "-1x368");
        pose_config.protoTxtPath = "../../depend/openpose/models/pose/body_25/pose_deploy.prototxt";
        pose_config.caffeModelPath = "../../depend/openpose/models/pose/body_25/pose_iter_584000.caffemodel";
        op::WrapperStructExtra extra_config {};

        op->configure(pose_config);
        op->configure(extra_config);
        op->disableMultiThreading();
        op->start();
    }

    void shutdown() {
        delete op;
    }
}
