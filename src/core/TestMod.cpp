#include <core/Node.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <memory>
#include <core/Module.h>
#include <common/Message.h>
#include <common/SubscriptionHandler.h>
#include <msgs/TestMsgs.h>


struct Context {
   uint32_t counter;
   Node* node;
   Publisher* pub;
   SubscriptionHandler* sub;
};


void handle_resp(void* ctx, TestRespMsg const* msg) {
    printf("Got resp %u!\n", msg->counter);
}

void handle_cmd(void* usr_ptr, TestReqMsg const* msg) {
    printf("Got cmd %u!\n", msg->counter);

    auto ctx = static_cast<Context*>(usr_ptr);

    MessageBuilder<TestRespMsg> resp_msg;
    resp_msg->counter = msg->counter;
    ctx->pub->publish(resp_msg);
}

extern "C" {

ModuleState* initialize(Node* node) {
    auto ctx = (Context*) malloc(sizeof(Context));
    ctx->counter = 0;
    ctx->node = node;

    ctx->sub = new SubscriptionHandler(node);

    if (node->id == 0) {
        printf("Initialize A\n");
        ctx->pub = node->advertise<TestReqMsg>("test_cmd");
        ctx->sub->subscribe<TestRespMsg>("test_resp", handle_resp, ctx);
    } else {
        printf("Initialize B\n");
        ctx->pub = node->advertise<TestRespMsg>("test_resp");
        ctx->sub->subscribe<TestReqMsg>("test_cmd", handle_cmd, ctx);
    }

    return ctx;
}

void tick(Node* node, Context* ctx) {

    ctx->sub->tick();

    if (node->id == 0) {

        MessageBuilder<TestReqMsg> msg;

        msg->counter = ctx->counter;
        ctx->pub->publish(msg);
        printf("Tick A %d\n", ctx->counter++);
    } else {
        printf("Tick B\n");
    }

}

void shutdown(Node* node, Context* ctx) {
    printf("Shutdown A\n");

    delete ctx->sub;

    free(ctx);
}

void unload(Node* node, Context* ctx) {
    printf("Unload A\n");
}

void reload(Node* node, Context* ctx) {
    printf("Reload A\n");
}

} //extern "C"
