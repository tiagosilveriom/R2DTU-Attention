#include "Window.h"
#include "../EmptyView.h"
#include <SDL2/SDL2_gfxPrimitives.h>
#include <msgs/AudioMsgs.h>

void tts_network(void* usr, const WavMsg* msg) {
    auto[payload, length] = Messages::access_blob(&msg->payload);
    printf("Got msg\n");
    Audio::play_raw(payload, length);
}

void terminal_cmd(void* usr, const TerminalCmdMsg* msg) {
    auto cmd_type = Messages::read_string(&msg->cmd_type);
    auto param = Messages::read_string(&msg->param);
    if (cmd_type == "audio" && param == "reset") {
        Audio::reset();
    } else if (cmd_type == "audio" && param == "check") {
        Audio::sound_check();
    }
}

Window::Window(const std::string& title, uint32_t width, uint32_t height, Node& node)
    : height_(height), width_(width) {

    sub = std::make_unique<SubscriptionHandler>(&node);
    sub->subscribe<WavMsg>("tts_network", &tts_network, nullptr);
    sub->subscribe<TerminalCmdMsg>("terminal_cmd", &terminal_cmd, nullptr);
    printf("Subscribing\n");

    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0) {
        printf("Failed to initialize SDL!\n");
        exit(-1);
    }


    if (TTF_Init() == -1) {
        printf("Failed to initialize SDL_TTF\n");
    }

    window_ = SDL_CreateWindow(title.c_str(), 0, 0, width, height, SDL_WINDOW_OPENGL);
    SDL_SetWindowBordered(window_, SDL_FALSE);

    renderer_ = SDL_CreateRenderer(window_, 0, SDL_RENDERER_ACCELERATED);

    register_view(std::make_unique<EmptyView>());

    for (uint32_t i = 0; i < NUM_VIEWPORTS; i++) {
        viewports_[i] = views_.front().get();
        viewports_[i]->inc_viewport();
    }

    top_bar_ = std::make_unique<TopBar>(this);

    Vec2i dropdown_offset = {10, 0};
    Vec2i dropdown_max_size = {PANEL_WIDTH, PANEL_HEIGHT};
    dropdown_context_ = std::make_unique<RenderContext>(renderer_, dropdown_offset, dropdown_max_size);

    state_cmd_pub = node.advertise<StateCmdMsg>("state_cmd");

    gamepad_.setup(node);
    Audio::setup();
}

Window::~Window() {
    SDL_DestroyWindow(window_);
    //Audio::shutdown();
    SDL_Quit();
}

void Window::register_view(std::unique_ptr<View>&& view) {
    view->setup(this);
    views_.push_back(move(view));
}


void Window::set_offset(uint32_t offset_x, uint32_t offset_y) {
    SDL_SetWindowPosition(window_, offset_x, offset_y);
}

Vec2i Window::calculate_viewport_pos(int i) {
    int x = PANEL_WIDTH * (i % NUM_COLS);
    int y = (PANEL_HEIGHT) * (i / NUM_COLS) + TOPBAR_HEIGHT;
    return {x, y};
}

void Window::draw_dropdown() {

    auto num_items = (int)views_.size();

    dropdown_context_->prerender();
    dropdown_height_ = num_items * DROPDOWN_LINE_HEIGHT;
    dropdown_context_->fill_box({0,0}, {PANEL_WIDTH, dropdown_height_ + DROPDOWN_LINE_HEIGHT}, Color::light_black());
    for (auto i = 0; i < num_items; i++) {
        dropdown_context_->draw_text(views_[i]->name(), {10, DROPDOWN_LINE_HEIGHT * i + 2}, TextAlign::LEFT, 18, Color::foreground());
    }
}

std::pair<int, Vec2i> Window::calculate_mouse_event_viewport(Vec2i pos) {
    for (auto i = 0; i < NUM_VIEWPORTS; i++) {

        auto [x1, y1] = calculate_viewport_pos(i);

        int x2 = VIEW_WIDTH + x1;
        int y2 = VIEW_HEIGHT + y1;

        int x = pos.x;
        int y = pos.y;

        if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
            return {i, {x - x1, y - y1}};
    }
    return {-1, pos};
}

void Window::set_viewport(int viewport, int view_id) {
    viewports_[viewport]->dec_viewport();
    viewports_[viewport] = views_[view_id].get();
    viewports_[viewport]->inc_viewport();
}

void Window::handle_dropdown_click(Vec2i pos) {
    auto item = (pos.y - TITLEBAR_HEIGHT) / DROPDOWN_LINE_HEIGHT;
    set_viewport(dropdown_viewport_, item);
    dropdown_viewport_ = -1;
}

void Window::handle_click(SDL_Event event) {
    auto [viewport, view_pos] = calculate_mouse_event_viewport({event.button.x, event.button.y});
    if (viewport == -1)
        return;
    if (viewport == dropdown_viewport_ && view_pos.y < (dropdown_height_ + TITLEBAR_HEIGHT)) {
        handle_dropdown_click(view_pos);
    } else if (view_pos.y < TITLEBAR_HEIGHT) {
        dropdown_viewport_ = viewport;
        draw_dropdown();
    } else {
        dropdown_viewport_ = -1;
        auto button = event.button.button == SDL_BUTTON_LEFT ? MouseButton::LEFT : MouseButton::RIGHT;
        viewports_[viewport]->parent_click(view_pos, button);
    }
}

bool Window::tick() {
    sub->tick();
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0x00);
    SDL_RenderClear(renderer_);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return false;
            case SDL_MOUSEBUTTONDOWN:
                handle_click(event);
                break;
            case SDL_JOYAXISMOTION:
                gamepad_.handle_axis(&event.jaxis);
                break;
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                gamepad_.handle_button(&event.jbutton);
                break;
            case SDL_JOYHATMOTION:
                gamepad_.handle_hat(&event.jhat);
                break;
            case SDL_JOYDEVICEREMOVED:
            case SDL_JOYDEVICEADDED:
                gamepad_.handle_device(&event.jdevice);
                break;
            case SDL_KEYUP:
            case SDL_KEYDOWN: {
                auto key = event.key.keysym;
                views_[active_view_]->handle_key(key, event.type);

                MessageBuilder<StateCmdMsg> msg;

                SDL_JoyButtonEvent fake;
                fake.timestamp = event.key.timestamp;
                fake.state = (event.type == SDL_KEYDOWN) ? 1 : 0;

                if (event.key.keysym.sym == SDLK_DOWN) {
                    fake.button = GAMEPAD_DDOWN;
                    gamepad_.handle_button(&fake);
                } else if (event.key.keysym.sym == SDLK_RIGHT) {
                    fake.button = GAMEPAD_DRIGHT;
                    gamepad_.handle_button(&fake);
                } else if (event.key.keysym.sym == SDLK_LEFT) {
                    fake.button = GAMEPAD_DLEFT;
                    gamepad_.handle_button(&fake);
                } else if (event.key.keysym.sym == SDLK_UP) {
                    fake.button = GAMEPAD_DUP;
                    gamepad_.handle_button(&fake);
                } else if (event.key.keysym.sym == SDLK_F12) {
                    fake.button = GAMEPAD_X;
                    gamepad_.handle_button(&fake);
                }


                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_F1) {
                        msg->type = StateCmdMsg::Confirm;
                        state_cmd_pub->publish(msg);
                    } else if (event.key.keysym.sym == SDLK_F2) {
                        msg->type = StateCmdMsg::Back;
                        state_cmd_pub->publish(msg);
                    } else if (event.key.keysym.sym == SDLK_F3) {
                        msg->type = StateCmdMsg::Next;
                        state_cmd_pub->publish(msg);
                    }
                }

            }
            default:
                break;
        }
    }

    top_bar_->render();

    for (auto& view : views_) {
        if (view->is_visible()) {
            view->parent_tick();
        }
    }

    for (uint32_t i = 0; i < NUM_VIEWPORTS; i++) {
        auto pos = calculate_viewport_pos(i);
        viewports_[i]->draw_to_screen(pos);
    }

    if (dropdown_viewport_ != -1) {
        auto pos = calculate_viewport_pos(dropdown_viewport_);
        pos.y += TITLEBAR_HEIGHT;
        dropdown_context_->postrender(pos, {PANEL_WIDTH, dropdown_height_ + DROPDOWN_LINE_HEIGHT});
    }

    SDL_RenderPresent(renderer_);

    return true;
}
