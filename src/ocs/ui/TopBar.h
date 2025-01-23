#pragma once

#include <memory>
#include "RenderContext.h"

class Window;

static const int TOPBAR_HEIGHT = 40;
static const int TOPBAR_WIDTH = 1920;

class TopBar {
public:
    TopBar(Window* window);

    void render();

private:
    Window* parent_;
    std::unique_ptr<RenderContext> context_;
};
