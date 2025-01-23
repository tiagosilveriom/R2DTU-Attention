#include "Perception.h"

#include <core/Node.h>
#include <core/Module.h>
#include <msgs/CameraMsgs.h>
#include <msgs/PerceptionMsgs.h>
#include <msgs/NaoMsgs.h>

#include <thread>

#include <opencv2/opencv.hpp>
#include <opencv2/freetype.hpp>
#include <librealsense2/rs.hpp>
#include <librealsense2/rsutil.h>
#include <atomic>
#include <acting/Trajectory.h>

#include <common/RingBuffer.h>

#include "sensors/Realsense.h"
#include "sensors/StreamSensor.h"
#include "sensors/LocalSensor.h"

#include "detectors/object_detection.h"
#include "detectors/marker_detection.h"
#include "person/person_async.h"

#include <glm/gtx/string_cast.hpp>
#include <common/SubscriptionHandler.h>
#include <msgs/ControlMsgs.h>


const uint32_t MARKER_FREQUENCY = 1;
const uint32_t FACE_FREQUENCY = 1;
const uint32_t TRACKING_FREQUENCY = 5;
const uint32_t SKELETON_FREQUENCY = 1;
const uint32_t OBJECT_FREQUENCY = 5;

glm::vec3 deproject_pixel(const Intrinsics& intr, glm::vec3 pixel) {

    float in[2];
    in[0] = (float) pixel.x;
    in[1] = (float) pixel.y;

    float out[3];

    rs2_intrinsics rs_intr;
    rs_intr.width = intr.width;
    rs_intr.height = intr.height;
    rs_intr.fx = intr.fx;
    rs_intr.fy = intr.fy;
    rs_intr.ppx = intr.cx;
    rs_intr.ppy = intr.cy;

    rs2_deproject_pixel_to_point(&out[0], &rs_intr, &in[0], pixel.z);

    glm::vec3 v {out[0], out[1], out[2]};
    return v;
}

glm::vec2 project_point_to_pixel(const Intrinsics& intr, glm::vec3 point) {
    
    float in[3];
    in[0] = point.x;
    in[1] = point.y;
    in[2] = point.z;

    rs2_intrinsics rs_intr;
    rs_intr.width = intr.width;
    rs_intr.height = intr.height;
    rs_intr.fx = intr.fx;
    rs_intr.fy = intr.fy;
    rs_intr.ppx = intr.cx;
    rs_intr.ppy = intr.cy;
    
    float out[2];
    rs2_project_point_to_pixel(&out[0], &rs_intr, &in[0]);
    
    glm::vec2 p {out[0], out[1]};
    return p;
}

static cv::Scalar bone_colours[] = {
        cv::Scalar(153, 0, 51),
        cv::Scalar(153, 0, 0),
        cv::Scalar(153, 51, 51),
        cv::Scalar(153, 102, 0),
        cv::Scalar(153, 153, 0),
        cv::Scalar(102, 153, 0),
        cv::Scalar(51, 153, 0),
        cv::Scalar(0, 153, 0),
        cv::Scalar(0, 102, 153),
        cv::Scalar(0, 153, 51),
        cv::Scalar(0, 153, 102),
        cv::Scalar(0, 153, 153),
        cv::Scalar(0, 51, 153),
        cv::Scalar(0, 0, 153),
        cv::Scalar(153, 0, 102),
        cv::Scalar(102, 0, 153),
        cv::Scalar(153, 0, 153),
        cv::Scalar(51, 0, 153),
};

static int bones[] = {
        1,2,
        2,3,
        3,4,
        1,5,
        5,6,
        6,7,
        6,7,
        1,8,
        8,9,
        9,10,
        10,11,
        8,12,
        12,13,
        13,14,
        15,17,
        16,18,
};

class Perception {
public:

    Perception(Node *node) : onboard_cam(node, "vis_forward"), aux_cam(0) {
        ocs_up_pub = node->advertise<ImageMsg>("operator_view_perception_1");
        ocs_down_pub = node->advertise<ImageMsg>("operator_view_perception_2");
        ocs_onboard_pub = node->advertise<ImageMsg>("operator_view_perception_3");
        ocs_depth_pub = node->advertise<ImageMsg>("operator_view_perception_4");
        ocs_aux_pub = node->advertise<ImageMsg>("operator_view_perception_5");
        percept_pub = node->advertise<PerceptsMsg>("percepts");
        rect_scan_resp_pub = node->advertise<RectScanRespMsg>("rect_scan_resp");

        up_cam.initialize("909512071727", false);
        down_cam.initialize("909512071681", true);

        sub = std::make_unique<SubscriptionHandler>(node);

        sub->subscribe<TerminalCmdMsg>("terminal_cmd", std::bind(&Perception::terminal_cmd, this, std::placeholders::_1, std::placeholders::_2),nullptr);
        sub->subscribe<RectScanCmdMsg>("rect_scan_cmd", std::bind(&Perception::handle_rect_scan_cmd, this, std::placeholders::_1, std::placeholders::_2),nullptr);

        font = cv::freetype::createFreeType2();
        font->loadFontData("../resources/B612-Bold.ttf", 0);

        MarkerDetection::initialize();
        PersonAsync::initialize();
        ObjectDetection::initialize();

	depth_buffer = cv::Mat(480, 640, CV_8UC3, cv::Scalar(0,0,0));

	    inject_dummy = false;
	    dummy_person.id = 100;
	    dummy_person.name = "Dummy";
	    dummy_person.skeleton.id = 1000;
        dummy_person.skeleton.certainty[0] = 1.0f;
        dummy_person.skeleton.joints[0] = glm::vec3(0,0,2);
        dummy_person.skeleton.raw_joints[0] = glm::vec2(0,0);
        for (int i = 1; i < 25; ++i) {
            dummy_person.skeleton.certainty[i] = 0.0f;
	    }
    }

    void terminal_cmd(void* usr, const TerminalCmdMsg* msg) {
        auto type = Messages::read_string(&msg->cmd_type);
        auto param = Messages::read_string(&msg->param);

        if (type == "startlearn") {
            auto id = atoi(param.c_str());
            PersonAsync::start_learn(id);
        } else if (type == "learn") {
            PersonAsync::learn(param);
        } else if (type == "delete") {
            PersonAsync::delete_person(param);
        } else if (type == "dummy") {
            inject_dummy = !inject_dummy;
        }
    }
    
    RectScanCmdMsg rect_scan;
    bool enable_rect_scan = false;
    
    void handle_rect_scan_cmd(void* usr, const RectScanCmdMsg* msg) {
        rect_scan = *msg;
        enable_rect_scan = msg->enable;
    }

    void send_percepts(int source_index, std::vector<MarkerPercept>& marker_percepts, std::vector<PersonPercept>& person_percepts, std::vector<ObjectPercept>& object_percepts) {
        MessageBuilder<PerceptsMsg> percepts_msg;
        percepts_msg->source_index = source_index;

        percepts_msg->markers = (MarkerPercept*) malloc(sizeof(MarkerPercept) * marker_percepts.size());
        percepts_msg->num_markers = marker_percepts.size();
        memcpy(percepts_msg->markers, &marker_percepts[0], sizeof(MarkerPercept) * marker_percepts.size());

        percepts_msg->persons = (PersonPercept*) malloc(sizeof(PersonPercept) * person_percepts.size());
        percepts_msg->num_persons = person_percepts.size();
        memcpy(percepts_msg->persons, &person_percepts[0], sizeof(PersonPercept) * person_percepts.size());

        percepts_msg->objects = (ObjectPercept*) malloc(sizeof(ObjectPercept) * object_percepts.size());
        percepts_msg->num_objects = object_percepts.size();
        memcpy(percepts_msg->objects, &object_percepts[0], sizeof(ObjectPercept) * object_percepts.size());


        marker_percepts.clear();
        person_percepts.clear();
        object_percepts.clear();
        
        percept_pub->publish(percepts_msg);
    }

    void send_percepts(int source_index, std::vector<MarkerPercept>& marker_percepts, std::vector<ObjectPercept>& object_percepts) {
        MessageBuilder<PerceptsMsg> percepts_msg;
        percepts_msg->source_index = source_index;

        percepts_msg->markers = (MarkerPercept*) malloc(sizeof(MarkerPercept) * marker_percepts.size());
        percepts_msg->num_markers = marker_percepts.size();
        memcpy(percepts_msg->markers, &marker_percepts[0], sizeof(MarkerPercept) * marker_percepts.size());

        percepts_msg->objects = (ObjectPercept*) malloc(sizeof(ObjectPercept) * object_percepts.size());
        percepts_msg->num_objects = object_percepts.size();
        memcpy(percepts_msg->objects, &object_percepts[0], sizeof(ObjectPercept) * object_percepts.size());

        percepts_msg->persons = nullptr;
        percepts_msg->num_persons = 0;

        marker_percepts.clear();
        object_percepts.clear();

        percept_pub->publish(percepts_msg);
    }  

    void send_percepts(int source_index, std::vector<MarkerPercept>& marker_percepts, std::vector<PersonPercept>& person_percepts) {
        MessageBuilder<PerceptsMsg> percepts_msg;
        percepts_msg->source_index = source_index;

        percepts_msg->markers = (MarkerPercept*) malloc(sizeof(MarkerPercept) * marker_percepts.size());
        percepts_msg->num_markers = marker_percepts.size();
        memcpy(percepts_msg->markers, &marker_percepts[0], sizeof(MarkerPercept) * marker_percepts.size());

        percepts_msg->persons = (PersonPercept*) malloc(sizeof(PersonPercept) * person_percepts.size());
        percepts_msg->num_persons = person_percepts.size();
        memcpy(percepts_msg->persons, &person_percepts[0], sizeof(PersonPercept) * person_percepts.size());

        percepts_msg->objects = nullptr;
        percepts_msg->num_objects = 0;

        marker_percepts.clear();
        person_percepts.clear();



        percept_pub->publish(percepts_msg);
    }


    void send_percepts(int source_index, std::vector<MarkerPercept>& marker_percepts) {
        MessageBuilder<PerceptsMsg> percepts_msg;
        percepts_msg->source_index = source_index;
        percepts_msg->markers = (MarkerPercept*) malloc(sizeof(MarkerPercept) * marker_percepts.size());
        percepts_msg->num_markers = marker_percepts.size();
        memcpy(percepts_msg->markers, &marker_percepts[0], sizeof(MarkerPercept) * marker_percepts.size());

        percepts_msg->persons = nullptr;
        percepts_msg->num_persons = 0;
        percepts_msg->objects = nullptr;
        percepts_msg->num_objects = 0;

        marker_percepts.clear();

        percept_pub->publish(percepts_msg);
    }

    void send_ocs_image(cv::Mat img, Publisher* pub) {
        MessageBuilder<ImageMsg> msg;

        cv::Mat small;
        if (img.cols != 640 || img.rows != 480) {
            cv::resize(img, small, cv::Size(640,480));
        } else {
            small = img;
        }

        msg->width = small.cols;
        msg->height = small.rows;

        auto len = small.cols * small.rows * small.channels();

        msg.write_blob(&msg->payload, small.data, len);

        pub->publish(msg);
    }

    void tick() {

        sub->tick();

        static uint32_t up_counter = 0;

        std::vector<MarkerPercept> marker_percepts;
        std::vector<PersonPercept> person_percepts;
        std::vector<ObjectPercept> object_percepts;

        Option<FrameSet> frame_poll;

        frame_poll = up_cam.poll_frames();

        /* Up Camera*/
        if (frame_poll) {
            FrameSet frames = *frame_poll;

            PersonAsync::put(frames);
            //if (up_counter % OBJECT_FREQUENCY == 0) ObjectDetection::put(frames);
            ++up_counter;

            ObjectDetection::detect(frames, object_percepts);
            MarkerDetection::detect(frames, marker_percepts);
            PersonAsync::take_all(person_percepts);
            if (inject_dummy) {
                person_percepts.push_back(dummy_person);
            }
            if (!person_percepts.empty()) {
                drawing_percepts = person_percepts;
                drawing_percept_timeout = 3;
            }

            if (drawing_percept_timeout-- <= 0) {
                drawing_percepts.clear();
            }



            cv::Mat drawing = frames.colour->clone();

            for (auto& marker : marker_percepts) {
                cv::circle(drawing, cv::Point(marker.raw_pos.x, marker.raw_pos.y), 30, cv::Scalar(0xFF, 0xFF, 0), 5);
            }
            //circle the center of the object
            /*
            for (auto& object : object_percepts) {
                cv::circle(drawing, cv::Point(object.raw_pos.x, object.raw_pos.y), 30, cv::Scalar(0xFF, 0xFF, 0), 5);
            }
            */

            // Draw object bounding boxes
            //std::cout << "Number of objects perceptioned:" << object_percepts.size() << "\n" ;
            
            for (const auto& object : object_percepts) {

                const cv::Rect2f& bbox = object.bounding_box;

                cv::rectangle(drawing, cv::Point(bbox.x, bbox.y), cv::Point(bbox.x + bbox.width, bbox.y + bbox.height), cv::Scalar(0, 255, 0), 2);

                std::string label = object.name;

                cv::putText(drawing, label, 
                            cv::Point(bbox.x, bbox.y - 10), 
                            cv::FONT_HERSHEY_SIMPLEX, 
                            2, cv::Scalar(0xFF, 0, 0), 3); // Red label          
            }

            for (auto& person : drawing_percepts) {
                auto& skeleton = person.skeleton;
                for (auto i = 0u; i < sizeof(bones) / (2*sizeof(bones[0])); ++i) {
                    auto from = skeleton.raw_joints[bones[2*i]];
                    auto to = skeleton.raw_joints[bones[2*i+1]];
                    if (fabs(from.x) + fabs(from.y) > 1.0 && fabs(to.x) + fabs(to.y) > 1.0) {
                        cv::line(drawing, cv::Point(from.x, from.y), cv::Point(to.x, to.y), bone_colours[bones[2*i]], 6);
                    }
                }
                for (int i = 1; i < 15; ++i) {
                    auto v = skeleton.raw_joints[i];
                    if (fabs(v.x) + fabs(v.y) < 1.0) continue;
                    cv::circle(drawing, cv::Point(v.x, v.y), 12, bone_colours[i], -1);
                }

                /*auto face_pos = skeleton.raw_joints[0];
                if (face_pos.x + face_pos.y > 1.0) {
                    cv::Scalar color = person.face ? cv::Scalar(0, 200, 0) : cv::Scalar(0,0,200);
                    cv::circle(drawing, cv::Point(face_pos.x, face_pos.y), 10, color, -1);
                }*/
            }

            //send_percepts(0, marker_percepts, person_percepts);
            send_percepts(0, marker_percepts, person_percepts, object_percepts);

            cv::Mat small_depth_up;
            cv::resize(*frames.depth_viz, small_depth_up, cv::Size(320, 240));

            small_depth_up.copyTo(depth_buffer(cv::Rect(120,0, 320, 240)));

            send_ocs_image(drawing, ocs_up_pub);
            send_ocs_image(depth_buffer, ocs_depth_pub);
        }

        frame_poll = down_cam.poll_frames();
        /* Down Camera*/
        if (frame_poll) {
            FrameSet frames = *frame_poll;

            MarkerDetection::detect(frames, marker_percepts);
            ObjectDetection::detect(frames, object_percepts);
            
            for (auto& marker : marker_percepts) {
                cv::circle(*frames.colour, cv::Point(marker.raw_pos.x, marker.raw_pos.y), 30, cv::Scalar(0xFF, 0xFF, 0), 5);
            }
            
            for (const auto& object : object_percepts) {

                const cv::Rect2f& bbox = object.bounding_box;

                cv::rectangle(*frames.colour, cv::Point(bbox.x, bbox.y), cv::Point(bbox.x + bbox.width, bbox.y + bbox.height), cv::Scalar(0, 255, 0), 2);

                std::string label = object.name;

                cv::putText(*frames.colour, label, 
                            cv::Point(bbox.x, bbox.y - 10), 
                            cv::FONT_HERSHEY_SIMPLEX, 
                            2, cv::Scalar(0xFF, 0, 0), 3); // Red label          
            }
            //send_percepts(1, marker_percepts);
            send_percepts(1, marker_percepts, object_percepts);

	        cv::Mat small_depth_down;
    	    cv::resize(*frames.depth_viz, small_depth_down, cv::Size(320, 240));

    	    small_depth_down.copyTo(depth_buffer(cv::Rect(120,240, 320, 240)));

            send_ocs_image(*frames.colour, ocs_down_pub);
        }

        frame_poll = onboard_cam.poll_frames();
        /* Board Camera*/
        if (frame_poll) {
            FrameSet frames = *frame_poll;

            MarkerDetection::detect(frames, marker_percepts);

            for (auto& marker : marker_percepts) {
                cv::circle(*frames.colour, cv::Point(marker.raw_pos.x, marker.raw_pos.y), 30, cv::Scalar(0xFF, 0xFF, 0), 5);
            }


            send_percepts(2, marker_percepts);

            send_ocs_image(*frames.colour, ocs_onboard_pub);
        }

        frame_poll = aux_cam.poll_frames();
        if (frame_poll) {
            FrameSet frames = *frame_poll;

            send_ocs_image(*frames.colour, ocs_aux_pub);
        }

    }

    ~Perception() {
        PersonAsync::shutdown();
        MarkerDetection::shutdown();
        ObjectDetection::shutdown();

    }

private:
    RealSense up_cam;
    RealSense down_cam;
    StreamSensor onboard_cam;
    LocalSensor aux_cam;

    std::unique_ptr<SubscriptionHandler> sub;

    Publisher* ocs_up_pub;
    Publisher* ocs_down_pub;
    Publisher* ocs_onboard_pub;
    Publisher* ocs_depth_pub;
    Publisher* ocs_aux_pub;

    Publisher* percept_pub;
    Publisher* trajectory_pub;

    Publisher* rect_scan_resp_pub;

    glm::mat4x4 transform_onboard;
    glm::mat4x4 transform_tripod;
    bool offset_determined = false;

    bool inject_dummy;
    PersonPercept dummy_person;

    std::vector<PersonPercept> drawing_percepts;
    uint32_t drawing_percept_timeout;

    cv::Mat depth_buffer;

    cv::Ptr<cv::freetype::FreeType2> font;
};


extern "C" {

ModuleState* initialize(Node* node) {
    return new Perception(node);
}

void tick(Node* node, Perception* vision) {
    vision->tick();
}

void shutdown(Node* node, Perception* vision) {
    delete vision;
}

void unload(Node* node, Perception* vision) {

}

void reload(Node* node, Perception* vision) {

}

} //extern "C"
