#include "View.h"
#include "Window.h"

View::View(std::string name)
    : name_(name) {}

void View::setup(Window* win) {
    parent_ = win;
    Vec2i offset = {0, TITLEBAR_HEIGHT};
    Vec2i size = {PANEL_WIDTH, PANEL_HEIGHT};
    context_ = std::make_unique<RenderContext>(parent_->renderer_, offset, size);
    create(*context_);
}

void View::parent_click(Vec2i pos, MouseButton button) {
    Vec2i view_pos = {pos.x, pos.y - TITLEBAR_HEIGHT};
    handle_click(view_pos, button);
}

void View::parent_tick() {
    context_->prerender();

    context_->draw_box({0, -TITLEBAR_HEIGHT}, {PANEL_WIDTH-1, PANEL_HEIGHT-1}, Color::black(), 1);
    context_->draw_line({0, 0}, {PANEL_WIDTH-1, 0}, Color::black(), 2);

    context_->draw_text(title(), {10, 2-TITLEBAR_HEIGHT}, TextAlign::LEFT, 12, Color::foreground());
    render(*context_);
}

void View::draw_to_screen(Vec2i pos) {
    context_->postrender(pos, {PANEL_WIDTH, PANEL_HEIGHT});
}

