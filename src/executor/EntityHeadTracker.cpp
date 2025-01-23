//
// Created by dissing on 12/2/19.
//

#include "EntityHeadTracker.h"

static const float CUBE_TRACKING_DISTANCE = 0.25f;
static const int CUBE_TRACKING_TIMEOUT = 8;
static const int HEAD_TRACKING_FREQUENCY = 3;
static const int HEAD_TRACKING_SWITCH_FREQUENCY = 50;

void EntityHeadTracker::register_triggers(World& world) {
    using namespace std::placeholders;
    world.add_trigger({
        WorldObjectType::Person,
        std::bind(&EntityHeadTracker::person_trigger, this, _1)
    });
    world.add_trigger({
        WorldObjectType::Movable,
        std::bind(&EntityHeadTracker::cube_trigger, this, _1)
    });
}

void EntityHeadTracker::tick(Robot& r) {
    if (counter % HEAD_TRACKING_FREQUENCY == 0) {
        if (!potential_cube_targets.empty()) {
            last_look_at_cube = 0;
            r.look_at(potential_cube_targets[0]->center);
        } else if (!potential_person_targets.empty() && (last_look_at_cube < 0 || last_look_at_cube > CUBE_TRACKING_TIMEOUT)) {

            size_t chosen = 0;

            if (potential_person_targets.size() == 2) {
                chosen = potential_person_targets[0]->name == current_track_target ? 0 : 1;

                if (person_switch_timer > HEAD_TRACKING_SWITCH_FREQUENCY) {
                    chosen = chosen == 0 ? 1 : 0;
                    person_switch_timer = 0;
                }
            }
            current_track_target = potential_person_targets[chosen]->name;
            r.look_at(potential_person_targets[chosen]->latest_pose.joints[0]);
            person_switch_timer++;
            last_look_at_cube = -1;
        }
        if (last_look_at_cube >= 0) {
            last_look_at_cube++;
        }
    }
    potential_person_targets.clear();
    potential_cube_targets.clear();
    counter++;
}

void EntityHeadTracker::person_trigger(WorldObject* obj) {
    PersonObj* person = (PersonObj*) obj;
    if (person->status == WorldObjectStatus::Tracked && !person->name.empty()) {
        potential_person_targets.push_back(person);
    }
}

void EntityHeadTracker::cube_trigger(WorldObject* obj) {
    MovableObj* movable = (MovableObj*) obj;
    if (movable->status == WorldObjectStatus::Hypothetical || movable->marker_id < 1000) return;

    for (auto& person : potential_person_targets) {
        auto left_valid = person->latest_pose.certainty[7] > 0.3;
        auto right_valid = person->latest_pose.certainty[4] > 0.3;
        auto distance_from_left_hand = glm::length(person->latest_pose.joints[7] - movable->center);
        auto distance_from_right_hand = glm::length(person->latest_pose.joints[4] - movable->center);
        if (left_valid && distance_from_left_hand < CUBE_TRACKING_DISTANCE) {
            potential_cube_targets.push_back(movable);
        }
        if (right_valid && distance_from_right_hand < CUBE_TRACKING_DISTANCE) {
            potential_cube_targets.push_back(movable);
        }
    }
    return;
}
