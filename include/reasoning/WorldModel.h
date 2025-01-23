#pragma once

#include <string>
#include <common/Option.h>
#include <perception/percept.h>
#include <core/Node.h>
#include <msgs/PerceptionMsgs.h>
#include <common/SubscriptionHandler.h>

const int SIGHTINGS_TO_LOCK_PERSON = 2;
const int TIMEOUT_LOCK_PERSON = 4;
const int TIMEOUT_LOSE_PERSON = 4;

const int SIGHTINGS_TO_LOCK_MARKER = 2;
const int TIMEOUT_LOCK_MARKER = 10;
const int TIMEOUT_LOSE_MARKER = 10;

const float HEAD_BODY_INCONSISTENCY_DISTANCE = 0.15;

enum class WorldObjectType {
    Person,
    Movable,
};

class WorldObject;

class WorldEvent {
public:

    enum Type {
        NewObject,
        Appear,
        Disappear
    };
    Type type;
    WorldObject* obj;

    static WorldEvent new_object(WorldObject* obj) {
        WorldEvent e = {NewObject, obj};
        return e;
    }

    static WorldEvent appear(WorldObject* obj) {
        WorldEvent e = {Appear, obj};
        return e;
    }

    static WorldEvent disappear(WorldObject* obj) {
        WorldEvent e = {Disappear, obj};
        return e;
    }
};

enum class WorldObjectStatus {
    Hypothetical,
    Tracked,
    Lost,
};

class World;

class WorldObject {
public:
    WorldObjectType type;
    WorldObjectStatus status;
    std::string name;

    int32_t timeout_counter;
    int32_t sightings_counter;
    int32_t needed_sightings;
    int32_t initial_lock_counter;
    int32_t initial_lose_counter;
    bool should_delete;
    bool inconsistent = false;

    WorldObject(WorldObjectType type, std::string name, int32_t needed_sightings, int32_t initial_lock_counter, int32_t initial_lose_counter)
            : type(type), status(WorldObjectStatus::Hypothetical), name(name), timeout_counter(0), sightings_counter(0),
              needed_sightings(needed_sightings), initial_lock_counter(initial_lock_counter), initial_lose_counter(initial_lose_counter), should_delete(false) {}

    virtual bool match(const MarkerPercept& percept) {return false; }
    virtual bool match(const PersonPercept& percept) { return false; }
    virtual bool match(const ObjectPercept& percept) { return false; }


    template <typename P>
    void run_update(World& world, const P& percept) {
        produce_sighting();
        update(world, percept);
    }

    Option<WorldEvent> tick(World& world);

protected:
    virtual void update(World& world, const MarkerPercept& percept) {}
    virtual void update(World& world, const PersonPercept& percept) {}
    virtual void update(World& world, const ObjectPercept& percept) {}


private:
    void produce_sighting();

};

class PersonObj;
class MovableObj;

class SensorSources {
public:
    enum Type {
        Active,
        Passive
    };
    SensorSources(int id, Type type);

    void calibrate(glm::mat4x4 source_transform);
    glm::vec3 camera_to_world_space(glm::vec3 v) const;
    glm::vec3 world_to_camera_space(glm::vec3 v) const;
    bool can_use_percepts() const { return calibrated && type == Active; }
    int should_calibrate() const { return !calibrated; }

    void request_calibration() { calibrated = false; }

private:
    glm::mat4x4 camera_to_world;
    glm::mat4x4 world_to_camera;
    int samples_collected;
    int id;
    Type type;
    bool calibrated;
};

struct WorldTrigger {
    WorldObjectType entity_type;
    std::function<void(WorldObject*)> f;
};

struct WorldPairTrigger {
    WorldObjectType fst_entity_type;
    WorldObjectType snd_entity_type;
    std::function<void(WorldObject*, WorldObject*)> f;
};

using WorldCustomTrigger = std::function<void(World&)>;

using EventTransformer = std::function<Option<WorldEvent>(World&, WorldEvent)>;

class World {
public:

    explicit World(Node& node);

    void process_percepts(void* ctx, const PerceptsMsg* percepts);

    template <typename P>
    void match_percepts(const P* percepts, size_t num_percepts);

    void add_trigger(WorldTrigger trigger) {
        single_triggers.push_back(std::move(trigger));
    }

    void add_trigger(WorldPairTrigger trigger) {
        pair_triggers.push_back(std::move(trigger));
    }

    void add_transformer(WorldEvent::Type type, EventTransformer transformer) {
        auto search = transformers.find(type);
        if (search != transformers.end()) {
            search->second.push_back(std::move(transformer));
        } else {
            std::vector<EventTransformer> transformer_vec = {transformer};
            transformers.insert({type, transformer_vec});
        }
    }

    void tick();

    const std::vector<WorldEvent>& get_latest_events() const {
        return latest_events;
    }

    const std::vector<std::unique_ptr<WorldObject>>& iter_all() const {
        return objects;
    }

    const std::vector<PersonObj*>& iter_persons() const {
        return person_objects;
    }

    const std::vector<MovableObj*>& iter_movables() const {
        return movable_objects;
    }

    void create_hypothesis(const MarkerPercept& percept) {
        if (percept.id == 0 || (percept.id > 4 && percept.id < 100) || (percept.id > 100 && percept.id <= 1000) || percept.id > 1004) return;
        auto obj = std::make_unique<MovableObj>(percept.id, "marker");
        ((WorldObject*)obj.get())->run_update(*this, percept);
        movable_objects.push_back(obj.get());
        objects.emplace_back(std::move(obj));
    }

    void create_hypothesis(const PersonPercept& percept) {
        auto obj = std::make_unique<PersonObj>(percept.id, percept.name);
        ((WorldObject*)obj.get())->run_update(*this, percept);
        person_objects.push_back(obj.get());
        objects.emplace_back(std::move(obj));
    }
    void create_hypothesis(const ObjectPercept& percept) {
        if (percept.type == PerceptionObjectType::Unknown) return;

        auto obj = std::make_unique<MovableObj>(percept.id, percept.name);
        ((WorldObject*)obj.get())->run_update(*this, percept);
        
        movable_objects.push_back(obj.get());
        objects.emplace_back(std::move(obj));
        
    }
    SensorSources& get_source(int id) {
        return sources.at(id);
    }

    void clear() {
        latest_events.clear();
        objects.clear();
        person_objects.clear();
        movable_objects.clear();
        new_percepts = false;
        single_triggers.clear();
        pair_triggers.clear();
        transformers.clear();
    }

    void pause(bool pause) {
        is_paused = pause;
    }

private:

    std::vector<WorldEvent> latest_events;

    std::vector<std::unique_ptr<WorldObject>> objects;
    std::vector<PersonObj*> person_objects;
    std::vector<MovableObj*> movable_objects;

    std::vector<WorldCustomTrigger> custom_triggers;
    std::vector<WorldTrigger> single_triggers;
    std::vector<WorldPairTrigger> pair_triggers;


    std::unordered_map<WorldEvent::Type, std::vector<EventTransformer >> transformers;

    std::unique_ptr<SubscriptionHandler> sub;

    std::unordered_map<int, SensorSources> sources;

    bool is_paused = false;

    bool new_percepts;
};

struct PersonObj : public WorldObject {

    explicit PersonObj(uint32_t id, std::string name) : WorldObject(WorldObjectType::Person, std::move(name), SIGHTINGS_TO_LOCK_PERSON, TIMEOUT_LOCK_PERSON, TIMEOUT_LOSE_PERSON), id(id) {}

    SkeletonPercept latest_pose;

    uint32_t id;

    bool match(const PersonPercept& percept) override;

    void update(World& world, const PersonPercept& percept) override ;

};

struct MovableObj : WorldObject {

/*
    explicit MovableObj(uint32_t id)
    : WorldObject(WorldObjectType::Movable, "marker-" + std::to_string(id), SIGHTINGS_TO_LOCK_MARKER, TIMEOUT_LOCK_MARKER, TIMEOUT_LOSE_MARKER), marker_id(id) {}
*/
    explicit MovableObj(uint32_t id, std::string name)
    : WorldObject(WorldObjectType::Movable, name + "-" + std::to_string(id), SIGHTINGS_TO_LOCK_MARKER, TIMEOUT_LOCK_MARKER, TIMEOUT_LOSE_MARKER), marker_id(id) {}

    glm::mat4x4 transform;
    glm::vec3 center;
    uint32_t marker_id; //marker_id refers to object id also
    PerceptionObjectType type;

    bool match(const MarkerPercept& percept) override {
        return marker_id == percept.id;
    }
    bool match(const ObjectPercept& percept) override { //TODO: Later it will be changed to allow to have more than 1 object per type 
        int pos1= name.find("-");
        int pos2= percept.name.find("-");
        return name.substr(0,pos1) == percept.name.substr(0,pos2);
    }
    virtual void update(World& world, const MarkerPercept& percept) {
        center = percept.pos;
        transform = percept.transform;
        type= PerceptionObjectType::Marker;
    }

    virtual void update(World& world, const ObjectPercept& percept) {
        center = percept.pos;
        type= percept.type;
    }
};
