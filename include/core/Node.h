#pragma once

#include <core/Types.h>
#include <common/Debug.h>

#include <memory>
#include <unordered_map>

#include <core/MPSCQueue.h>
#include <common/Message.h>
#include "KVStore.h"

struct MessageHeader;

struct Module;

class Publisher;

class Subscriber {
public:
    ~Subscriber() = default;

    void mark_as_networked() { networked_ = true; }

    void unsubscribe() {

    }

    explicit Subscriber(Publisher* pub) : owner_(pub), queue_(DEFAULT_QUEUE_SIZE) {}

    void push(std::shared_ptr<MessageHeader> msg) {
        ASSERT(msg.get() != nullptr, "STORING NULL MSG\n");
        queue_.put(msg);
    }

    Option<std::shared_ptr<MessageHeader>> next() {
        auto msg = queue_.get();
        if (msg.is_some() && *msg == nullptr) {
            ASSERT(false, "GOT NULL MSG FROM QUEUE\n")
        }
        return msg;
    }

    friend class Node;

private:
    const static size_t DEFAULT_QUEUE_SIZE = 16;
    Publisher* owner_;
    MPSCQueue<std::shared_ptr<MessageHeader>> queue_;
    bool networked_ = false;

};


class Publisher {
public:

    Publisher(const std::type_info& type_info, uint32_t channel_id) : type_info_(type_info), channel_id_(channel_id) {}

    template <typename Msg>
    void publish(MessageBuilder<Msg>& msg) {
        ASSERTF(typeid(Msg) == type_info_, "Tried to publish msg of type %s on channel %u with type %s\n", typeid(Msg).name(), channel_id_, type_info_.name());
        auto header = msg.pack();
        for (auto& sub : subscribers_) {
            sub->push(header);
        }
    }

    void publish(std::shared_ptr<MessageHeader>& header) {
        for (auto& sub : subscribers_) {
            sub->push(header);
        }
    }

    Subscriber* add_subscriber(std::string const& topic) {
        auto ptr = new Subscriber(this);
        subscribers_.push_back(std::unique_ptr<Subscriber> {ptr});
        return ptr;
    }

    friend class Node;

private:
    std::vector<std::unique_ptr<Subscriber>> subscribers_;
    const std::type_info& type_info_;
    uint32_t channel_id_;
};

/**
 * The Node functions as the system API for the modules.
 * It allows modules to interact with the pub/sub system and get access to other shared services
 * All functions are therefore thread-safe.
 */
class Node {

public:

    //Module management

    void (*load_module)(const char* name, uint32_t frequency);

    //Communication

    /**
     * advertise announces the intention to publish messages of the given type on the specified channel.
     * The user can them publish messages using the returned handle
     * If the channel already exists with another message type, this method will panic at runtime with an error message.
     */
    template <typename Msg>
    Publisher* advertise(std::string const& topic) {
        std::lock_guard<std::recursive_mutex> lock(monitor_mutex_);
        auto search = publishers_.find(topic);
        if (search != publishers_.end()) {
            auto& type = typeid(Msg);
            ASSERTF(search->second->type_info_ == type, "\"%s\" on topic \"%s\" of type \"%s\"\n", type.name(), topic.c_str(), search->second->type_info_.name());
            return search->second.get();
        } else {

            uint32_t id;
            auto opt = kv.zscore("topic:topics", topic);
            if (opt.is_some()) {
                id = *opt;
            } else {
                id = kv.incr("topic:next_id");
                kv.zadd(KVStore::NX, "topic:topics", topic, id);
                auto new_opt = kv.zscore("topic:topics", topic);
                ASSERT(new_opt.is_some(), "Failed to add topic to Key Value store\n");
                id = *new_opt;
            }

            publishers_[topic] = std::make_unique<Publisher>(typeid(Msg), id);
            auto pub = publishers_.at(topic).get();
            publishers_by_channel_.insert({id, pub});
            return pub;
        }
    }

    /**
     * subscribe announces the interest in recieving messages of the given type from the specified channel
     * The returned subscription handle gives access to a MPSC queue where the received messages will be placed.
     * For a callback based API, use SubscriptionHandler instead.
     * If the channel already exists with another message type, this method will panic at runtime with an error message.
     * @return
     */
    template <typename Msg>
    Subscriber* subscribe(std::string const& topic) {
        std::lock_guard<std::recursive_mutex> lock(monitor_mutex_);
        auto pub = advertise<Msg>(topic);
        return pub->add_subscriber(topic);
    }

    //Temporary functions until the leader channel discovery is implemented
    template <typename Msg>
    void network_advertise(std::string const& topic) {
        std::lock_guard<std::recursive_mutex> lock(monitor_mutex_);
        auto sub = subscribe<Msg>(topic);
        sub->mark_as_networked();
        network_subs_.push_back(sub);
    }


    //Execution management
    //These are not for module consumption and should only be called from main()

    /**
     * Must be called before the use of any other Node API calls
     * The parameters are temporary until a leader is added
     * @param config_name
     * @param is_leader
     */
    void initialize(std::string const& config_name, bool is_leader, const char* leader_ip);

    //Temporary function to signal that all nodes have started
    void start();

    /**
     * Should be called after spin has returned to ensure propper cleanup of resources
     * and Lifecycle communication with other nodes
     */
    void shutdown();

    /**
     * Node inner loop. Will not return until a shutdown signal has been received
     */
    void spin();

    //The KV store allows access to shared redis instance
    //Used for configuration

    KVStore& access_kv() {
        return kv;
    }

    uint8_t id;

private:

    void internal_tick();
    void network_tick();

private:

    std::recursive_mutex monitor_mutex_;
    std::vector<Subscriber*> network_subs_;
    std::unordered_map<std::string, std::unique_ptr<Publisher>> publishers_;
    std::unordered_map<uint32_t , Publisher*> publishers_by_channel_;

    KVStore kv;

};

