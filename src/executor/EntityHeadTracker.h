#pragma once

#include <reasoning/WorldModel.h>
#include "robot.h"

class EntityHeadTracker {
public:
    void register_triggers(World& world);
    void tick(Robot& r);

private:
    void person_trigger(WorldObject* person);
    void cube_trigger(WorldObject* cube);

    uint32_t counter = 0;
    int last_look_at_cube;
    std::string current_track_target;
    int person_switch_timer;

    std::vector<PersonObj*> potential_person_targets;
    std::vector<MovableObj*> potential_cube_targets;
};

