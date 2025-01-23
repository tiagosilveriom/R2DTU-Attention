#pragma once

#include "../Perception.h"
#include <opencv2/core.hpp>
#include "perception/percept.h"

namespace SkeletonDetection {

    void initialize();

    std::vector<SkeletonPercept> detect(FrameSet frames);

    void shutdown();
}