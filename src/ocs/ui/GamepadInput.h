#pragma once

#include <SDL2/SDL.h>
#include <core/Node.h>
#include <msgs/ControlMsgs.h>

class GamepadInput {
public:
    bool setup(Node& node);

    void handle_axis(SDL_JoyAxisEvent* event);
    void handle_button(SDL_JoyButtonEvent* event);
    void handle_device(SDL_JoyDeviceEvent* event);
    void handle_hat(SDL_JoyHatEvent* event);

private:
    Publisher* gamepad_axis_pub_;
    Publisher* gamepad_button_pub_;
    Uint8 current_hat_ = 0;
};

