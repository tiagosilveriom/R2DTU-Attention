#include <opencv2/opencv.hpp>
#include "CameraView.h"

void CameraView::handle_image(void* usr_ptr, const ImageMsg* msg) {
    auto[data_ptr, data_len] = Messages::access_blob(&msg->payload);
    video_texture_->upload(data_ptr, data_len);
}

void CameraView::handle_text_draw(void* usr_ptr, const TextDrawingMsg* msg) {
    auto text = Messages::read_string(&msg->text);
    if (text.empty()) return;
    auto search = draw_objects.find(text);
    if (search == draw_objects.end()) {
        draw_objects.insert({text, {}});
    }
    DrawObject& obj = draw_objects[text];
    obj.lifetime = 3;
    obj.text = text;
    obj.pos = Vec2i {msg->x, msg->y};
}

CameraView::CameraView(Node& node, std::string channel)
  : View("Camera: " + channel), video_texture_(nullptr), channel_(channel) {
    sub = std::make_unique<SubscriptionHandler>(&node);
    sub->subscribe<ImageMsg>(channel, std::bind(&CameraView::handle_image, this, std::placeholders::_1, std::placeholders::_2), nullptr);
    sub->subscribe<TextDrawingMsg>(channel + "_text_drawing", std::bind(&CameraView::handle_text_draw, this, std::placeholders::_1, std::placeholders::_2), nullptr);
}

void CameraView::create(RenderContext& context) {
    video_texture_ = new Texture(context, Texture::RGB24, Texture::STREAMING, 640, 480);
}

std::string CameraView::title() {
    return channel_;
}

void CameraView::render(RenderContext& context) {
    sub->tick();
    context.draw_texture(video_texture_, {0,0});
    for (auto it = draw_objects.begin(); it != draw_objects.end(); ) {
        auto& obj = it->second;
        if (obj.lifetime-- == 0) {
            it = draw_objects.erase(it);
        } else {
            context.draw_text(obj.text, obj.pos, TextAlign::LEFT, 58, Color::white(), 3, Color::background());
            it++;
        }
    }
}
