#include <core/Node.h>
#include <core/Module.h>
#include "ui/Window.h"
#include "CameraView.h"
#include "TelemetryView.h"
#include "FalseBeliefView.h"
#include "ScriptRunnerView.h"
#include "TerminalView.h"

struct Context {
    Window* window;
};

extern "C" {

ModuleState* initialize(Node* node) {
    auto ctx = (Context*) malloc(sizeof(Context));
    ctx->window = new Window("R2DTU Operator Control Station", 1920, 1040, *node);

    ctx->window->register_view(std::unique_ptr<View> {new CameraView(*node, "operator_view_perception_1") });
    ctx->window->register_view(std::unique_ptr<View> {new CameraView(*node, "operator_view_perception_2") });
    ctx->window->register_view(std::unique_ptr<View> {new CameraView(*node, "operator_view_perception_3") });
    ctx->window->register_view(std::unique_ptr<View> {new CameraView(*node, "operator_view_perception_4") });
    ctx->window->register_view(std::unique_ptr<View> {new FalseBeliefView(*node) });
    ctx->window->register_view(std::unique_ptr<View> {new TerminalView(*node) });
    ctx->window->register_view(std::unique_ptr<View> {new TelemetryView(*node) });
    ctx->window->register_view(std::unique_ptr<View> {new CameraView(*node, "operator_view_perception_5") });


    ctx->window->set_viewport(0, 1);
    ctx->window->set_viewport(3, 2);
    ctx->window->set_viewport(4, 3);
    ctx->window->set_viewport(5, 4);
    ctx->window->set_viewport(1, 5);
    ctx->window->set_viewport(2, 6);
    ctx->window->set_active(6);

    /*for (int i = 0; i < 4; ++i) {
        ctx->window->set_viewport(i, i);
    }*/
    return ctx;
}

void tick(Node* node, Context* ctx) {
    ctx->window->tick();
}

void shutdown(Node* node, Context* ctx) {

}

void unload(Node* node, Context* ctx) {

}

void reload(Node* node, Context* ctx) {

}

} //extern "C"
