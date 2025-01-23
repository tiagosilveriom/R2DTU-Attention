//
// Created by s154250 on 9/26/19.
//

#include "person_async.h"
#include "person_tracker.h"
#include <common/RingBuffer.h>
#include <atomic>
#include <thread>

namespace PersonAsync {

    struct Context {
        RingBuffer<FrameSet> frame_buffer;
        RingBuffer<PersonPercept> percept_buffer;
        std::atomic<bool> shutdown_flag;
        std::thread worker;

        uint32_t start_learn_id;
        std::atomic<bool> start_learn_flag;
        std::string learn_name;
        std::atomic<bool> learn_flag;
        std::string delete_name;
        std::atomic<bool> delete_flag;

        Context() : frame_buffer(4), percept_buffer(16), shutdown_flag(false), start_learn_flag(false), learn_flag(false), delete_flag(false) {}
    };

    Context* ctx;

    void initialize() {
        ctx = new Context();
        PersonTracker::initialize();

        ctx->worker = std::thread([]() {
            std::vector <PersonPercept> percepts;
            while (!ctx->shutdown_flag) {
                if (ctx->start_learn_flag) {
                    PersonTracker::start_learn(ctx->start_learn_id);
                    ctx->start_learn_flag = false;
                }

                if (ctx->learn_flag) {
                    PersonTracker::learn(ctx->learn_name);
                    ctx->learn_flag = false;
                }

                if (ctx->delete_flag) {
                    PersonTracker::delete_person(ctx->delete_name);
                    ctx->delete_flag = false;
                }

                auto frames = ctx->frame_buffer.take();
                PersonTracker::process(frames, percepts);
                ctx->percept_buffer.clear();
                ctx->percept_buffer.put_all(percepts);
                percepts.clear();
            }
        });
    }

    void start_learn(uint32_t id) {
        ctx->start_learn_id = id;
        ctx->start_learn_flag = true;
    }

    void learn(std::string name) {
        ctx->learn_name = name;
        ctx->learn_flag = true;
    }

    void delete_person(std::string name) {
        ctx->delete_name = name;
        ctx->delete_flag = true;
    }

    void put(FrameSet frames) {
        ctx->frame_buffer.put(frames);
    }

    void take_all(std::vector<PersonPercept>& percepts) {
        ctx->percept_buffer.take_all(percepts);
    }

    void shutdown() {
        ctx->shutdown_flag = true;
        ctx->worker.join();
        PersonTracker::shutdown();
        delete ctx;
    }
}