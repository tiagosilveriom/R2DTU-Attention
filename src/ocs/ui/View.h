#pragma once

#include <string>
#include <memory>
#include "RenderContext.h"

const int TITLEBAR_HEIGHT = 20;
const int VIEW_WIDTH = 640;
const int VIEW_HEIGHT = 480;
const int PANEL_WIDTH = VIEW_WIDTH;
const int PANEL_HEIGHT = VIEW_HEIGHT + TITLEBAR_HEIGHT;

class Window;

enum class MouseButton {
    LEFT,
    RIGHT
};

class View {
public:

    View(std::string name);
    virtual ~View() = default;


    virtual void create(RenderContext& context) = 0;

    virtual std::string title() = 0;
    virtual void render(RenderContext& context) = 0;

    virtual bool handle_click(Vec2i pos, MouseButton button) { return false; };

    virtual void handle_key(SDL_Keysym key, uint32_t type) {};


    void parent_click(Vec2i pos, MouseButton button);
    void parent_tick();
    void setup(Window* win);

    void draw_to_screen(Vec2i pos);

    std::string& name() {
        return name_;
    }

    bool is_visible() const {
        return num_viewports > 0;
    }

    void inc_viewport() {
        num_viewports++;
    }

    void dec_viewport() {
        num_viewports--;
    }

private:
    Window* parent_;
    std::unique_ptr<RenderContext> context_;

    int num_viewports = 0;
    std::string name_;
};

