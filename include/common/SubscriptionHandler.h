#pragma once

#include <vector>
#include <memory>
#include <functional>

#include <core/Node.h>

struct Node;
struct MessageHeader;


class SubscriptionHandler {

public:

    SubscriptionHandler(Node* node) : node_(node) {}

    ~SubscriptionHandler() {
        for (auto& sub : subs_) {
            sub.subscriber->unsubscribe();
        }
    }

    template <typename Msg>
    using Callback = std::function<void(void*, Msg const*)>;

    using CallbackWrapper = std::function<void(std::shared_ptr<MessageHeader>, void*)>;

    template <typename Msg>
    void subscribe(std::string topic, Callback<Msg> callback, void* ctx) {
        Subscription sub;
        sub.subscriber = node_->subscribe<Msg>(topic);
        sub.callback = [callback](std::shared_ptr<MessageHeader> msg, void* usr_ptr) {
            callback(usr_ptr, Messages::content<Msg>(msg.get()));
        };
        sub.usr_ptr = ctx;
        sub.topic = topic;
        subs_.push_back(sub);
    }

    void unsubscribe(std::string const& topic) {
        for (auto iter = subs_.begin(); iter != subs_.end(); ++iter) {
            if (topic == iter->topic) {
                iter->subscriber->unsubscribe();
                subs_.erase(iter);
                return;
            }
        }
        fprintf(stderr, "Tried to unsubscribe from unknown topic %s\n", topic.c_str());
    }

    void tick() {
        for (auto& sub : subs_) {

            auto opt = sub.subscriber->next();

            while (opt.is_some()) {
                sub.callback(*opt, sub.usr_ptr);

                opt = sub.subscriber->next();
            }
        }
    }

private:

    struct Subscription {
        Subscriber* subscriber;
        CallbackWrapper callback;
        void* usr_ptr;
        std::string topic;
    };

    Node* node_;
    std::vector<Subscription> subs_;

};
