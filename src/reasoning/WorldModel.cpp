#include <reasoning/WorldModel.h>

#include <core/Node.h>

#include <vector>
#include <perception/percept.h>
#include <memory>
#include <msgs/PerceptionMsgs.h>
#include <algorithm>
#include <unordered_set>
#include <common/SubscriptionHandler.h>

#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

Option<WorldEvent> WorldObject::tick(World &world) {
    auto event = Option<WorldEvent>();
    if (status == WorldObjectStatus::Hypothetical) {
        if (sightings_counter >= needed_sightings) {
            printf("New object appear %s\n", name.c_str());
            //Produce new object event
            event = Option<WorldEvent>(WorldEvent::new_object(this));
            status = WorldObjectStatus::Tracked;
            timeout_counter = initial_lose_counter;
        } else if (timeout_counter <= 0) {
            should_delete = true;
        }
    } else if (status == WorldObjectStatus::Tracked) {
        if (timeout_counter <= 0 || inconsistent) {
            //Produce disappear event
            printf("Object disappear %s\n", name.c_str());
            event = Option<WorldEvent>(WorldEvent::disappear(this));
            status = WorldObjectStatus::Lost;
            sightings_counter = 0;
            inconsistent = false;
        }
    } else if (status == WorldObjectStatus::Lost) {
        if (sightings_counter >= needed_sightings) {
            //Produce appear event
            printf("Object appear %s\n", name.c_str());
            event = Option<WorldEvent>(WorldEvent::appear(this));
            status = WorldObjectStatus::Tracked;
            timeout_counter = initial_lose_counter;
        } else if (timeout_counter <= 0) {
            sightings_counter = 0;
        }
    }
    timeout_counter = std::max(timeout_counter - 1, 0);
    return event;
}

void WorldObject::produce_sighting() {
    if (status == WorldObjectStatus::Lost || status == WorldObjectStatus::Hypothetical) {
        if (sightings_counter == 0) {
            timeout_counter = initial_lock_counter;
        }
        sightings_counter++;
    } else if (status == WorldObjectStatus::Tracked) {
        timeout_counter = initial_lose_counter;
    }
}

bool PersonObj::match(const PersonPercept& percept) {
    return (std::string(percept.name).empty() && percept.id == id) || percept.name == name;
}

void PersonObj::update(World& world, const PersonPercept& percept) {
    name = percept.name;
    id = percept.id;
    latest_pose = percept.skeleton;
}

World::World(Node& node) : sub(std::make_unique<SubscriptionHandler>(&node)) {
    sub->subscribe<PerceptsMsg>("percepts",  std::bind(&World::process_percepts, this, std::placeholders::_1, std::placeholders::_2), nullptr);
    sources.insert({0, SensorSources(0, SensorSources::Active)});
    sources.insert({1, SensorSources(1, SensorSources::Active)});
    sources.insert({2, SensorSources(2, SensorSources::Passive)});
}

template <typename P>
void World::match_percepts(const P* percepts, size_t num_percepts) {
    
    std::unordered_set<size_t> unused_percepts;
    for (auto j = 0u; j < num_percepts; ++j) {
        bool match = false;
        for (auto i = 0u; i < objects.size(); ++i) {
            if (objects[i]->match(percepts[j])) { //TODO: match for Object will have to be changed
                match = true;
                objects[i]->run_update(*this, percepts[j]);
                break;
            }
        }
        if (!match) unused_percepts.insert(j);
    }
    
    for (auto i : unused_percepts) {
        create_hypothesis(percepts[i]);
    }
}

void transform_percepts(const SensorSources& source, MarkerPercept* percepts, size_t num_percepts) {
    for (auto i = 0u; i < num_percepts; ++i) {
        percepts[i].pos = source.camera_to_world_space(percepts[i].pos);
    }
}


void transform_percepts(const SensorSources& source, PersonPercept* percepts, size_t num_percepts) {
    for (auto i = 0u; i < num_percepts; ++i) {
        for (auto j = 0u; j < POSE_JOINTS; ++j) {
            if (percepts[i].skeleton.certainty[j] > 0.0) {
                percepts[i].skeleton.joints[j] = source.camera_to_world_space(percepts[i].skeleton.joints[j]);
            }
        }
    }
}
void transform_percepts(const SensorSources& source, ObjectPercept* percepts, size_t num_percepts) {
    for (auto i = 0u; i < num_percepts; ++i) {
        percepts[i].pos= source.camera_to_world_space(percepts[i].pos);
    }
}

void World::process_percepts(void* ctx, const PerceptsMsg* percepts) { /* method to process perceptions*/
    if (is_paused) return;

    auto& source = sources.at(percepts->source_index);

    if (source.can_use_percepts()) {
        new_percepts = true;
        transform_percepts(source, percepts->markers, percepts->num_markers);
        match_percepts(percepts->markers, percepts->num_markers);
        transform_percepts(source, percepts->persons, percepts->num_persons);
        match_percepts(percepts->persons, percepts->num_persons);
        transform_percepts(source, percepts->objects, percepts->num_objects);
        match_percepts(percepts->objects, percepts->num_objects); //TODO: match_percepts for objects has to be changed to handle more than 1 objet from the same type
    }

    if (source.should_calibrate()) {
        for (auto i = 0u; i < percepts->num_markers; ++i) {
            if (percepts->markers[i].id == 0) {
                source.calibrate(percepts->markers[i].transform);
                break;
            }
        }
    }

    free(percepts->markers);
    free(percepts->persons);
    free(percepts->objects);

}

void World::tick() {
    if (is_paused) return;

    latest_events.clear();

    sub->tick();

    if (new_percepts) {
        for (auto i = 0u; i < objects.size(); ++i) {
            auto event = objects[i]->tick(*this);
            if (event) {
                latest_events.push_back(*event);
            }
            if (objects[i]->should_delete) {
                std::swap(objects[i], objects.back());
                objects.pop_back();
                continue;
            }
            for (auto& trig : single_triggers) {
                if (objects[i]->type == trig.entity_type) {
                    trig.f(objects[i].get());
                }
            }
        }

        for (auto& trig : custom_triggers) {
            trig(*this);
        }

        for (auto i = 0u; i < latest_events.size(); ++i) {
            auto& e = latest_events[i];
            auto search = transformers.find(e.type);
            if (search != transformers.end()) {
                for (auto& t : search->second) {
                    auto output = t(*this, e);
                    if (output) {
                        latest_events.push_back(*output);
                    }
                }
            }
        }

        for (auto& obj_i : objects) {
            for (auto& obj_j : objects) {
                if (obj_i == obj_j)  continue;

                for (auto& trig : pair_triggers) {
                    if (trig.fst_entity_type == obj_i->type && trig.snd_entity_type == obj_j->type) {
                        trig.f(obj_i.get(), obj_j.get());
                    }
                }
            }
        }

        new_percepts = false;
    }
}

SensorSources::SensorSources(int id, SensorSources::Type type) : camera_to_world(0.0f), world_to_camera(0.0f),
    samples_collected(0), id(id), type(type), calibrated(false)  {}

void SensorSources::calibrate(glm::mat4x4 source_transform) {
    if (glm::length(source_transform[3]) < 0.1) return;

    if (samples_collected < 20) {
        camera_to_world += source_transform;
        samples_collected++;
    } else {
        camera_to_world = camera_to_world * (1.0f/20.0f);
        printf("%s\n", glm::to_string(camera_to_world).c_str());
        world_to_camera = glm::inverse(camera_to_world);
        printf("%s\n", glm::to_string(world_to_camera).c_str());
        calibrated = true;
        printf("Source calibrated!\n");
    }
}

glm::vec3 SensorSources::camera_to_world_space(glm::vec3 v) const {
    return (world_to_camera * glm::vec4(v, 1.0f));
}

glm::vec3 SensorSources::world_to_camera_space(glm::vec3 v) const {
    return (camera_to_world * glm::vec4(v, 1.0f));
}
