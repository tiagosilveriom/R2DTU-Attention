//
// Created by dissing on 8/6/19.
//

#include "marker_detection.h"

#include <opencv2/opencv.hpp>
#include <apriltags/apriltag.h>
#include <apriltags/tagStandard41h12.h>
#include <apriltags/apriltag_pose.h>
#include <glm/gtx/string_cast.hpp>

namespace MarkerDetection {

    struct Context {
        apriltag_detector_t* detector;
        apriltag_family_t* family;
    };

    static Context ctx;

    void initialize() {
        ctx.detector = apriltag_detector_create();
        ctx.family = tagStandard41h12_create();
        apriltag_detector_add_family(ctx.detector, ctx.family);
    }

    zarray_t* find_all_markers(cv::Mat img) {
        cv::Mat gray;
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

        image_u8 z_img = {
                .width = gray.cols,
                .height = gray.rows,
                .stride = gray.cols,
                .buf = gray.data
        };

        return apriltag_detector_detect(ctx.detector, &z_img);
    }

    void detect(FrameSet frames, std::vector<MarkerPercept>& percepts) {
        if (!frames.colour.is_some()) return;

        apriltag_detection_info_t info;
        info.fx = frames.intrinsics.fx;
        info.fy = frames.intrinsics.fy;
        info.cx = frames.intrinsics.cx;
        info.cy = frames.intrinsics.cy;

        auto detections = find_all_markers(*frames.colour);

        for (int i = 0; i < zarray_size(detections); ++i) {
            apriltag_detection_t *detection;
            zarray_get(detections, i, &detection);

            MarkerPercept percept;
            percept.id = detection->id;

            if (detection->id == 0) {
                info.det = detection;
                info.tagsize = 0.093;

                apriltag_pose_t pose1;
                double err1;
                apriltag_pose_t pose2;
                double err2;
                estimate_tag_pose_orthogonal_iteration(&info, &err1, &pose1, &err2, &pose2, 50);

                apriltag_pose_t pose = (err1 < err2) ? pose1 : pose2;

                glm::mat4x4 transform(0.0);
                for (auto r = 0u; r < pose.R->nrows; ++r) {
                    for (auto c = 0u; c < pose.R->ncols; ++c) {
                        transform[c][r] = pose.R->data[r * pose.R->ncols + c];
                    }
                }
                for (auto r = 0u; r < pose.t->nrows; ++r) {
                    transform[3][r] = pose.t->data[r * pose.t->ncols];
                }
                transform[3][3] = 1;

                matd_destroy(pose1.R);
                matd_destroy(pose1.t);
                if (pose2.R) {
                    matd_destroy(pose2.t);
                }
                matd_destroy(pose2.R);

                percept.transform = transform;
            }


            glm::vec3 v;
            glm::vec2 raw_pos;
            raw_pos.x = v.x = detection->c[0];
            raw_pos.y = v.y = detection->c[1];
            if (frames.depth) {
                v.z = (float) (frames.depth->at<uint16_t>(v.y, v.x) * 0.001);
                percept.pos = deproject_pixel(frames.intrinsics, v);
            }
            percept.raw_pos = raw_pos;

            bool already_found = false;

            for (auto &other : percepts) {
                if (other.id == percept.id) {
                    already_found = true;
                    /*glm::vec4 z_axis = {0.0f, 0.0f, 1.0f, 0.0f};
                    auto score_existing = glm::dot(other.transform * z_axis, z_axis);
                    auto score_new = glm::dot(percept.transform * z_axis, z_axis);
                    if (score_new > score_existing) {
                        other.pos = percept.pos;
                        other.transform = percept.transform;
                    }*/
                }
            }
            if (!already_found) {
                percepts.push_back(percept);
            }
        }

        apriltag_detections_destroy(detections);
    }


    void shutdown() {
        apriltag_detector_destroy(ctx.detector);
        tagStandard41h12_destroy(ctx.family);
    }
}
