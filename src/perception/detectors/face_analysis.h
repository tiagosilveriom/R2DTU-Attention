#pragma once

#include "../Perception.h"
#include <opencv2/core/core.hpp>
#include "perception/percept.h"


typedef void* FaceEmbedding;

namespace FaceAnalysis {
    void initialize();

    FaceEmbedding* detect_single(cv::Mat img);

    float compare_embeddings(FaceEmbedding* a, FaceEmbedding* b);

    void shutdown();
}

