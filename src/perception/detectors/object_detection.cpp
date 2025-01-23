//
// Created by dissing on 8/6/19.
//

#include "object_detection.h"

namespace ObjectDetection {

    static network* net = nullptr;
    // Common parameters
    const float thresh = 0.65;
    const float hier_thresh = 0.5;
    float nms = .3;    // 0.4F Non-maximum-suppression
    int num_objects=0;

    std::vector<std::string> class_names;

    void initialize() {

        //const char *model_cfg = "../depend/darknet/cfg/yolov4-tiny.cfg";
        //const char* weights = "../depend/darknet/yolov4-tiny.weights";

        const char *model_cfg = "../depend/darknet/cfg/yolov4.cfg";
        const char* weights = "../depend/darknet/yolov4.weights";
        FILE* names_fp = fopen("../depend/darknet/data/coco.names", "r");
        if (!names_fp) {
            std::cerr << "Error: Cannot open names file!" << std::endl;
        }
        char buffer[256];
        while (fgets(buffer, 256, names_fp)) {
            buffer[strcspn(buffer, "\n")] = 0;  // Remove newline
            class_names.push_back(std::string(buffer));
        }
        fclose(names_fp);

        //TODO: improve the detection model
        net = load_network_custom(const_cast<char*>(model_cfg), const_cast<char*>(weights), 0, 1);

        calculate_binary_weights(*net);

    }
        
    void detect(FrameSet frames, std::vector<ObjectPercept>& percepts) {
        if (!frames.colour.is_some()) {
            std::cerr << "No colour frame available." << std::endl;
            return;
        }
        cv::Mat frame = *frames.colour;
        image darknet_img = mat_to_image(*frames.colour);

        image sized = resize_image(darknet_img, net->w, net->h);

        float* X = sized.data;

        //double time = get_time_point();
        network_predict(*net, X);
        //printf("Predicted in %lf milli-seconds.\n", ((double)get_time_point() - time) / 1000);

        int nboxes = 0;
        detection* dets = get_network_boxes(net, darknet_img.w, darknet_img.h, thresh, hier_thresh, 0, 1, &nboxes, 0);
        
        layer l = net->layers[net->n - 1];

        do_nms_sort(dets, nboxes, l.classes, nms);
        // Draw detections on the image
        for (int i = 0; i < nboxes; ++i) {
            for (int j = 0; j < dets[i].classes; ++j) {
                if (dets[i].prob[j] > thresh) {
                    std::string label = class_names[j];  // Map index to label
                    //float confidence = dets[i].prob[j];
                    box b = dets[i].bbox;
                    int x = (b.x - b.w / 2) * frame.cols;
                    int y = (b.y - b.h / 2) * frame.rows;
                    int w = b.w * frame.cols;
                    int h = b.h * frame.rows;

                    glm::vec3 v; // Bounding box center world space
                    glm::vec2 raw_pos; // Bounding box center camera space

                    raw_pos.x=v.x= x+w/2;
                    raw_pos.y=v.y=y+h/2;
/*
                    // Draw bounding box
                    cv::rectangle(frame, cv::Point(x, y), cv::Point(x + w, y + h), cv::Scalar(0, 255, 0), 2);

                    // Draw label with confidence
                    std::string text = label + " (" + std::to_string(confidence * 100).substr(0, 4) + "%)";
                    cv::putText(frame, text, cv::Point(x, y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);

                    // Console output

                    std::cout << "Detected: " << label
                            << " with confidence: " << confidence
                            << " at box: (" << x << ", " << y << ", " << w << ", " << h << ")" << std::endl;
*/
                    ObjectPercept perception ;

                    /*TODO: find how to make each object with its own ID */
                    //perception.id = percepts.size() + 101 ; // Objects id start from 100 
                    perception.name = label; // Set object label
                    perception.bounding_box = cv::Rect2f(x, y, w, h); // Set bounding box
                    if (frames.depth) {
                        v.z = (float) (frames.depth->at<uint16_t>(v.y, v.x) * 0.001);
                        perception.pos = deproject_pixel(frames.intrinsics, v); // Set object center world space (3D)
                    }
                    perception.raw_pos=raw_pos; // Set object center camera space (2D)

                    perception.type= get_object_type(perception.name); //Set object_type
                    perception.id = get_object_id(perception.name); //TODO: find a way for each object to have to have own ID and not hardcoed

                    // TODO: Add affordances based on objected detected
/*                    bool already_found = false;

                    for (auto &other : percepts) {
                        if(other.name != perception.name) // TODO: For now, we only add one unti of each type of obejct. Change Later to also investigate object's position
                        {
                            already_found=true;
                            break;
                        }
                    }
                    if(!already_found) */
                    percepts.push_back(perception); // Add to percepts array

                }
            }
        }
        //std::cout<< "Detected: "<< nboxes;

        // Free memory
        //free_detections(dets, nboxes);
        free_image(darknet_img);
        
    }




    void shutdown() {
        free_network(*net);
    }

    image mat_to_image(const cv::Mat& mat) {
        int h = mat.rows;
        int w = mat.cols;
        int c = mat.channels();
        image im = make_image(w, h, c);

        unsigned char* data = mat.data;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                for (int k = 0; k < c; ++k) {
                    im.data[k * w * h + y * w + x] = data[y * mat.step + x * c + k] / 255.0f;
                }
            }
        }
        return im;
    }

    PerceptionObjectType get_object_type(std::string object_name)
    {
        if(object_name=="knife") return PerceptionObjectType::Knife;
        else if(object_name=="apple") return PerceptionObjectType::Fruit;
        else if (object_name=="bottle") return PerceptionObjectType::Bottle;
        else if (object_name=="cup") return PerceptionObjectType::Cup;
        else return PerceptionObjectType::Unknown;
    }
    uint32_t get_object_id(std::string object_name)
    {
        if(object_name=="knife") return 104;
        else if(object_name=="apple") return 105;
        else if (object_name=="bottle") return 106;
        else if (object_name=="cup") return 107;
        else return 108; //108 is ID for Uknown
    }
    std::vector<std::string> get_object_affordances(PerceptionObjectType type)
    {
        switch(type)
        {
            case PerceptionObjectType::Knife:  return {"cutting"};
            case PerceptionObjectType::Fruit:  return {"eatable","cuttable"};
            case PerceptionObjectType::Bottle:  return {"pouring","fillable"};
            case PerceptionObjectType::Cup:    return {"drinking","fillable"};
            default: return {};
                                     
        }
    /*
        if(object_name=="knife") return {"cutting"};
        else if(object_name=="apple") return {"eatable","cuttable"};
        else if (object_name=="bottle") return {"pouring","fillable"};
        else if (object_name=="cup") return {"drinking","fillable"};
        else return {};
        */
    }


}


