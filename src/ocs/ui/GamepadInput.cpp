#include "GamepadInput.h"

bool GamepadInput::setup(Node& node) {

    gamepad_axis_pub_ = node.advertise<GamepadAxisMsg>("gamepad_axis");
    gamepad_button_pub_ = node.advertise<GamepadButtonMsg>("gamepad_button");

    auto num = SDL_NumJoysticks();
    if (num == 0) {
        return false;
    }

    auto joy = SDL_JoystickOpen(0);

    if (joy) {
        printf("Found game controller: %s\n", SDL_JoystickNameForIndex(0));
        printf("Number of Axes: %d\n", SDL_JoystickNumAxes(joy));
        printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(joy));
        printf("Number of Hats: %d\n", SDL_JoystickNumHats(joy));
    } else {
        return false;
    }

    return true;
}

void GamepadInput::handle_axis(SDL_JoyAxisEvent* event) {
    auto msg = MessageBuilder<GamepadAxisMsg>();

    msg->timestamp = event->timestamp;
    msg->axis_id = event->axis;
    msg->value = event->value;

    gamepad_axis_pub_->publish(msg);
}

void GamepadInput::handle_button(SDL_JoyButtonEvent* event) {
    auto msg = MessageBuilder<GamepadButtonMsg>();

    msg->timestamp = event->timestamp;
    msg->button_id = event->button;
    msg->state = event->state;

    gamepad_button_pub_->publish(msg);
}

void GamepadInput::handle_device(SDL_JoyDeviceEvent* event) {
    printf("Gamepad device event: %d -> %u\n", event->which, event->type);
}

void GamepadInput::handle_hat(SDL_JoyHatEvent* event) {


    for (uint8_t i = 0; i < 4; i++) {
        auto mask = (1 << i);
        auto cur_val = (current_hat_ & mask);
        auto new_val = (event->value & mask);
        if (cur_val ^ new_val) {
            auto msg = MessageBuilder<GamepadButtonMsg>();

            msg->timestamp = event->timestamp;
            msg->button_id = i + 14;
            msg->state = new_val != 0;

            gamepad_button_pub_->publish(msg);
        }
    }

    current_hat_ = event->value;

}
