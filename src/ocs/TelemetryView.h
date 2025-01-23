#pragma once

#include <core/Node.h>
#include <common/SubscriptionHandler.h>
#include <common/RobotDefs.h>
#include "ui/View.h"

const std::unordered_map<uint8_t , std::string> headers = {
        { JOINT_HEAD_PITCH, "HEAD" },
        { JOINT_HIP_PITCH, "LOWER BODY" },
        { JOINT_L_ELBOW_ROLL, "LEFT ARM" },
        { JOINT_R_ELBOW_ROLL, "RIGHT ARM" },
};

struct NodeTelemetry {
    std::string name;
    std::string ip;
    int cpu; //percentage usage
    int cpu_temp; //highest core temp
    int ram; //megabytes free
    int bat; //battery percentage; -1 means no battery
};


class TelemetryView : public View {
public:
    TelemetryView(Node& node);

    virtual void create(RenderContext& context) override;
    virtual std::string title() override;
    virtual void render(RenderContext& context) override;

    std::unordered_map<uint8_t , std::pair<int, float>> joints_;
    std::vector<NodeTelemetry> nodes_;

private:
    void draw_joint_item(RenderContext& context, uint8_t joint, int offset);
    int draw_node(RenderContext& context, const NodeTelemetry& node, int offset);
private:
    std::unique_ptr<SubscriptionHandler> sub_;

};


