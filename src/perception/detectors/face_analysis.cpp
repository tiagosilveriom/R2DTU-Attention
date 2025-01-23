//
// Created by dissing on 8/6/19.
//

#include "face_analysis.h"
#include <opencv2/opencv.hpp>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>
#include <dlib/image_io.h>
#include <iostream>

#include <face_recognition_model.hpp>
 
namespace FaceAnalysis {

    struct Context {
        frontal_face_detector face_detector;
        shape_predictor pose_model;
        face_recognition_model_v1* face_encoder;
    };

    static Context ctx;

    dlib::matrix<dlib::rgb_pixel> cv_img_to_dlib(cv::Mat img) {
        cv::Mat rgb;

        dlib::matrix<dlib::rgb_pixel> dimg; 
        dlib::assign_image(dimg, dlib::cv_image<dlib::rgb_pixel>(img));
        return dimg;
    }

    void initialize() {
        ctx.face_detector = get_frontal_face_detector();
        deserialize("../depend/face_recognition_models/face_recognition_models/models/shape_predictor_68_face_landmarks.dat") >> ctx.pose_model;
        std::string face_mode_path = "../depend/face_recognition_models/face_recognition_models/models/dlib_face_recognition_resnet_model_v1.dat";
        ctx.face_encoder = new face_recognition_model_v1(face_mode_path);
    }

    float compare_embeddings(FaceEmbedding* a, FaceEmbedding* b) {
        auto da = (dlib::matrix<float,0, 1>*) a;
        auto db = (dlib::matrix<float,0, 1>*) b;
        return length(*da - *db);
    }

    FaceEmbedding* detect_single(cv::Mat img) {

        auto dimg = cv_img_to_dlib(img);
        auto faces = ctx.face_detector(dimg);

        if (faces.size() == 1) {
            auto shape = ctx.pose_model(dimg, faces[0]);
            FaceEmbedding* embedding = (FaceEmbedding*) new dlib::matrix<float,0, 1>(ctx.face_encoder->compute_face_descriptor(dimg, shape));
            return embedding;
        }

        return nullptr;
    }

    void shutdown() {

    }

}
