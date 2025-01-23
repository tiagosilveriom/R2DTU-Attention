#pragma once

#include <stdint.h>
#include <memory>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <common/Color.h>
#include <common/SubscriptionHandler.h>
#include <vector>
#include "View.h"
#include "RenderContext.h"
#include "TopBar.h"
#include "GamepadInput.h"
#include "Audio.h"

class Window {
public:

    Window(const std::string& title, uint32_t width, uint32_t height, Node& node);
    ~Window();

    void register_view(std::unique_ptr<View>&& view);

    bool tick();

    void set_viewport(int viewport, int view_id);

    void set_active(int view) { active_view_ = view; }

    void set_offset(uint32_t offset_x, uint32_t offset_y);

    friend class View;
    friend class TopBar;

private:
    void draw_dropdown();


    Vec2i calculate_viewport_pos(int i);
    std::pair<int, Vec2i> calculate_mouse_event_viewport(Vec2i pos);

    void handle_click(SDL_Event event);
    void handle_dropdown_click(Vec2i pos);

private:
    SDL_Renderer* renderer_;
    SDL_Window* window_;
    uint32_t height_, width_;
    std::vector<std::unique_ptr<View>> views_;

    GamepadInput gamepad_;

    std::unique_ptr<TopBar> top_bar_;

    static const int DROPDOWN_LINE_HEIGHT = 20;
    int dropdown_viewport_ = -1;
    int dropdown_height_ = -1;
    std::unique_ptr<RenderContext> dropdown_context_;

    static const int NUM_VIEWPORTS = 6;
    static const int NUM_COLS = 3;
    View* viewports_[NUM_VIEWPORTS];

    Publisher* state_cmd_pub;

    std::unique_ptr<SubscriptionHandler> sub;

    int active_view_ = 0;
};
