#include "EmptyView.h"

std::string EmptyView::title() {
    return "Empty";
}

void EmptyView::render(RenderContext &context) {
    context.draw_text("Nothing here...", {PANEL_WIDTH/2, PANEL_HEIGHT/2}, TextAlign::CENTER, 24, Color::light_black());
}

