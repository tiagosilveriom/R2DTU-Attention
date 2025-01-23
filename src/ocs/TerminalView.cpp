#include "TerminalView.h"

#include <algorithm>
#include <msgs/ControlMsgs.h>


TerminalView::TerminalView(Node &node) : View("Terminal"), sub(std::make_unique<SubscriptionHandler>(&node)) {
    cmd_pub = node.advertise<TerminalCmdMsg>("terminal_cmd");
    sub->subscribe<TerminalOverrideMsg>("terminal_override", std::bind(&TerminalView::handle_terminal_override, this, std::placeholders::_1, std::placeholders::_2), this);
    sub->subscribe<LoggingMsg>("logging", std::bind(&TerminalView::handle_logging, this, std::placeholders::_1, std::placeholders::_2), this);
}

void TerminalView::create(RenderContext& context) {
}

std::string TerminalView::title() {
    return "Terminal";
}

static std::string level_names[3] = {"(ERROR)", "(WARN)", ""};

void TerminalView::handle_logging(void* usr, const LoggingMsg* msg) {
    std::string source = Messages::read_string(&msg->source);
    std::string text = Messages::read_string(&msg->text);
    char timestamp[64];
    struct tm* time_info = localtime(&msg->timestamp);
    strftime(timestamp, 64, "%M-%S: ", time_info);
    std::string combined = timestamp + text;
    log_buffer_.push_back(combined);
}

void TerminalView::handle_terminal_override(void* usr, const TerminalOverrideMsg* msg) {
    std::string text = Messages::read_string(&msg->text);
    cmd_buffer_ = text;
}

const int UTF16_O = 248;
const int UTF16_A = 229;
const int UTF16_AE = 230;

void TerminalView::handle_key(SDL_Keysym key, uint32_t type) {
    if (type == SDL_KEYDOWN) {
        if ((key.sym >= 'a' && key.sym <= 'z') || (key.sym >= '0' && key.sym <= '9')) {
            if (key.mod & KMOD_SHIFT) {
                cmd_buffer_.push_back((char) key.sym - ('a' - 'A'));
            } else {
                cmd_buffer_.push_back((char) key.sym);
            }
        } else if (key.sym == '.') {
            if (key.mod & KMOD_SHIFT) {
                cmd_buffer_.push_back(':');
            } else {
                cmd_buffer_.push_back('.');
            }
        } else if (key.sym == UTF16_A) {
                if (key.mod & KMOD_SHIFT) {
                    cmd_buffer_.push_back((char) 0xC3);
                    cmd_buffer_.push_back((char) 0x85);
                } else {
                    cmd_buffer_.push_back((char) 0xC3);
                    cmd_buffer_.push_back((char) 0xA5);
                }
            }
        } else if (key.sym == UTF16_O) {
            if (key.mod & KMOD_SHIFT) {
                cmd_buffer_.push_back((char) 0xC3);
                cmd_buffer_.push_back((char) 0x98);
            } else {
                cmd_buffer_.push_back((char) 0xC3);
                cmd_buffer_.push_back((char) 0xB8);
            }
        } else if (key.sym == UTF16_AE) {
            if (key.mod & KMOD_SHIFT) {
                cmd_buffer_.push_back((char) 0xC3);
                cmd_buffer_.push_back((char) 0x86);
            } else {
                cmd_buffer_.push_back((char) 0xC3);
                cmd_buffer_.push_back((char) 0xA6);
            }
        } else if (key.sym == SDLK_BACKSPACE && !cmd_buffer_.empty()) {
            if (cmd_buffer_.size() >= 2 && cmd_buffer_[cmd_buffer_.size() - 2] == (char) 0xC3) {
                cmd_buffer_.pop_back();
            }
            cmd_buffer_.pop_back();
        } else if (key.sym == SDLK_SPACE) {
            cmd_buffer_.push_back(' ');
        } else if (key.sym == SDLK_RETURN) {
            MessageBuilder<TerminalCmdMsg> msg;

	    std::string cmd;
	    std::string param = "";

            auto split = cmd_buffer_.find(' ');
            if (split != std::string::npos) {
                cmd = cmd_buffer_.substr(0, split);
                param = cmd_buffer_.substr(split+1);
            } else {
		cmd = cmd_buffer_;
	    }
	    msg.write_string(&msg->cmd_type, cmd);
            msg.write_string(&msg->param, param);
            cmd_pub->publish(msg);
            cmd_buffer_.clear();

        } else if (key.sym == SDLK_F5) {
            MessageBuilder<TerminalCmdMsg> msg;
	    std::string cmd = "sr";
	    msg.write_string(&msg->cmd_type, cmd);
	    msg.write_string(&msg->param, cmd);
	    cmd_pub->publish(msg);
    }
}

void TerminalView::render(RenderContext& context) {
    sub->tick();

    int current_line = 0;

    const int LINES = 11;

    int start = std::max(0, ((int) log_buffer_.size()) - LINES);

    for (auto i = start; i < (int) log_buffer_.size(); ++i) {
        auto& log = log_buffer_[i];
        context.draw_text(log, {24, current_line * 32}, TextAlign::LEFT, 32, Color::foreground());
        current_line++;
    }

    std::vector<std::string> cmd_buffer_draw;
    int last_space = -1;
    int last_cut = 0;
    for (int i = 0; i < (int) cmd_buffer_.size(); ++i) {
        if (cmd_buffer_[i] == ' ') last_space = i;

        if ((i - last_cut) > 40) {
            if (last_space == -1) last_space = i; //If we have not found any spaces on this line, we just cut right here
            cmd_buffer_draw.push_back(cmd_buffer_.substr(last_cut, last_space - last_cut));
            last_cut = last_space;
            i = last_space;
            last_space = -1;
        }
    }
    cmd_buffer_draw.push_back(cmd_buffer_.substr(last_cut, cmd_buffer_.size() - last_cut));

    int top = 460 - cmd_buffer_draw.size() * 32;

    for (int i = 0u; i < (int) cmd_buffer_draw.size(); ++i) {
        context.draw_text(cmd_buffer_draw[i], {32, top + i * 32}, TextAlign::LEFT, 32, Color::foreground());
    }
}
