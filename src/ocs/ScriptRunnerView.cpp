#include "ScriptRunnerView.h"

ScriptRunnerView::ScriptRunnerView(Node& node)
 : View("Script Runner"), runner_(node) {

}

void ScriptRunnerView::create(RenderContext &context) {

}

std::string ScriptRunnerView::title() {
    return "Script Runner";
}

void draw_current_item(RenderContext& context, const ScriptItem& item) {
    context.draw_text(item.short_desc(), {320, 160}, TextAlign::CENTER, 48, Color::foreground());

    auto long_desc = item.long_desc();
    for (auto i = 0; i < (int) long_desc.size(); ++i) {
        context.draw_text(long_desc[i], Vec2i{320, 200 + i * 30}, TextAlign::CENTER, 24, Color::foreground());
    }
}

void draw_previous_item(RenderContext& context, const ScriptItem& item) {
    Vec2i offset = {20,350};
    context.draw_text("Previous", Vec2i{0, 0} + offset, TextAlign::LEFT, 32, Color::foreground());
    context.draw_text(item.short_desc(), Vec2i{20, 40} +offset, TextAlign::LEFT, 24, Color::foreground());
}

void draw_next_item(RenderContext& context, const ScriptItem& item) {
    Vec2i offset = {620,350};
    context.draw_text("Next", Vec2i{0, 0} + offset, TextAlign::RIGHT, 28, Color::foreground());
    context.draw_text(item.short_desc(), Vec2i{-20, 40} +offset, TextAlign::RIGHT, 24, Color::foreground());
}

void draw_status_bar(RenderContext& context, ScriptRunner& runner) {
    auto current = std::to_string(runner.current_item_id() + 1);
    auto total = std::to_string(runner.total_items());
    context.draw_text(current + "/" + total, {20,20}, TextAlign::LEFT, 20, Color::foreground());
    context.draw_text(ScriptRunner::state_to_string(runner.current_state()), {320,30}, TextAlign::CENTER, 28, Color::foreground());
    auto time = runner.running_time();
    auto seconds = time % 60;
    auto minutes = time / 60;
    auto separator = minutes < 10 ? ":" : ":0";
    auto time_str = "T+"+std::to_string(minutes) + separator + std::to_string(seconds);
    context.draw_text(time_str, {500,20}, TextAlign::LEFT, 20, Color::foreground());
}

void ScriptRunnerView::render(RenderContext& context) {

    runner_.tick();

    draw_status_bar(context, runner_);

    auto current_id = runner_.current_item_id();
    if (current_id > 0) {
        draw_previous_item(context, runner_.get_item(current_id - 1));
    }
    if (current_id + 1 < runner_.total_items()) {
        draw_next_item(context, runner_.get_item(current_id + 1));
    }
    draw_current_item(context, runner_.get_item(current_id));
}


