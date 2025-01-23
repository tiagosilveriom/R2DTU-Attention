#pragma once

#include <core/Node.h>
#include <common/SubscriptionHandler.h>
#include <msgs/ControlMsgs.h>
#include <msgs/CameraMsgs.h>
#include "ui/View.h"

class CameraView : public View {
public:
    CameraView(Node& node, std::string channel);

    virtual void create(RenderContext& context) override;
    virtual std::string title() override;
    virtual void render(RenderContext& context) override;

    void handle_text_draw(void* usr, const TextDrawingMsg* msg);
    void handle_image(void* usr, const ImageMsg* msg);

private:
    Texture* video_texture_;
    std::string channel_;
    std::unique_ptr<SubscriptionHandler> sub;

    struct DrawObject {
        uint32_t lifetime;
        std::string text;
        Vec2i pos;
    };

    std::unordered_map<std::string, DrawObject> draw_objects;
};
