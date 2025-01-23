#include <sstream>
#include "TelemetryView.h"
#include <msgs/ProprioceptiveMsgs.h>

void handle_msg(void* usr_ptr, const HealthTelemetryMsg* msg) {
    auto self = static_cast<TelemetryView*>(usr_ptr);

    for (auto i = 0u; i < NUMBER_OF_JOINTS; i++) {
        self->joints_[i].first = msg->joint_temps[i];
        self->joints_[i].second = msg->joint_amps[i];
    }

    self->nodes_[1].bat = msg->battery;
    self->nodes_[1].cpu_temp = msg->cpu_temp;
    self->nodes_[1].cpu = msg->cpu_usage;
    self->nodes_[1].ram = msg->ram;

}

TelemetryView::TelemetryView(Node &node)
 : View("Telemetry") {
    for (int i = 0; i < 17; i++) {
        joints_.insert({i, {0, 0.0f}});
    }

    sub_ = std::make_unique<SubscriptionHandler>(&node);
    sub_->subscribe<HealthTelemetryMsg>("health_telemetry", handle_msg, this);

    nodes_.push_back({"Field", "192.168.0.101", 0, 0, 0, 0});
    nodes_.push_back({"Onboard", "192.168.0.102", 0, 0, 0, 0});
}

void TelemetryView::create(RenderContext& context) {

}

std::string TelemetryView::title() {
    return "Proprioceptive Telemetry";
}

void TelemetryView::draw_joint_item(RenderContext& context, uint8_t joint, int offset) {
    auto [temp, amp] = joints_.at(joint);
    const auto& name = joint_names.at(joint);
    context.draw_text(name, {10, offset}, TextAlign::LEFT, 12, Color::foreground());
    context.draw_text(std::to_string(temp) + " °C", {150, offset}, TextAlign::LEFT, 12, Color::foreground());

    amp = 0.0;
    //The horror of C++ streams. Unfortunately this is the "Correct" way to do it until fmt is standardized
    std::ostringstream amp_str;
    amp_str.precision(2);
    amp_str << std::fixed << amp << " A";
    context.draw_text(amp_str.str(), {200, offset}, TextAlign::LEFT, 12, Color::foreground());
}

int TelemetryView::draw_node(RenderContext& context, const NodeTelemetry& node, int offset) {
    int left_x = 400;
    context.draw_text(node.name, {left_x, offset}, TextAlign::LEFT, 14, Color::foreground());
    context.draw_text("IP:   " + node.ip, {left_x, offset+20}, TextAlign::LEFT, 12, Color::foreground());
    context.draw_text("CPU:  " + std::to_string(node.cpu) + "%", {left_x, offset+40}, TextAlign::LEFT, 12, Color::foreground());
    context.draw_text("Temp: " + std::to_string(node.cpu_temp) + " °C", {left_x, offset+60}, TextAlign::LEFT, 12, Color::foreground());
    context.draw_text("Free: " + std::to_string(node.ram) + " Mb", {left_x, offset+80}, TextAlign::LEFT, 12, Color::foreground());
    if (node.bat == -1) {
        return 100;
    }
    context.draw_text("Bat:  " + std::to_string(node.bat) + "%", {left_x, offset+100}, TextAlign::LEFT, 12, Color::foreground());
    return 120;
}

void TelemetryView::render(RenderContext& context) {
    sub_->tick();
    int offset_y = 0;
    for (int i = 0; i < 17; i++) {

        auto header = headers.find(i);
        if (header != headers.end()) {
            offset_y += 10;
            context.draw_text(header->second, {10, offset_y}, TextAlign::LEFT, 14, Color::foreground());
            offset_y += 20;
        }
        draw_joint_item(context, i, offset_y);
        offset_y += 20;
    }
    offset_y = 0;
    for (const auto& node : nodes_) {
        offset_y += draw_node(context, node, offset_y) + 10;
    }
}