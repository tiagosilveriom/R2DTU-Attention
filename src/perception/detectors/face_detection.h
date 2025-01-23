#pragma once
#include <perception/percept.h>
#include "../Perception.h"

namespace FaceDetection {

    void initialize();

    void put(FrameSet frames);

    void take_all(std::vector<FacePercept>& percepts);

    std::vector<FacePercept> drawings();

    void start_learn();

    void finish_learn(std::string name);

    void delete_person(std::string name);

    void shutdown();
}
