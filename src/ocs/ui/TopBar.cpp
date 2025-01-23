#include "TopBar.h"
#include "Window.h"

const Vec2i ORIGIN = {0,0};
const Vec2i SIZE = {TOPBAR_WIDTH, TOPBAR_HEIGHT};

TopBar::TopBar(Window* window)
    : parent_(window) {
    context_ = std::make_unique<RenderContext>(parent_->renderer_, ORIGIN, SIZE);
}

void TopBar::render() {
    context_->prerender();

    context_->fill_box(ORIGIN, SIZE, Color::black());

    context_->postrender(ORIGIN, SIZE);
}