#pragma once

#include "../Perception.h"
#include "perception/percept.h"

namespace PersonAsync {

    void initialize();

    void put(FrameSet frames);

    void take_all(std::vector<PersonPercept>& percepts);

    void start_learn(uint32_t id);

    void learn(std::string name);

    void delete_person(std::string name);

    void shutdown();
}
