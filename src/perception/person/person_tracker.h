#pragma once

#include <perception/percept.h>
#include <vector>
#include "../Perception.h"

namespace PersonTracker {
    void initialize();

    void process(FrameSet frame_set, std::vector<PersonPercept>& percepts);

    void start_learn(uint32_t id);

    void learn(std::string name);

    void delete_person(std::string name);

    void shutdown();
}

