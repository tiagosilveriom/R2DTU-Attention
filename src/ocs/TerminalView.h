#pragma once

#include <core/Node.h>
#include <common/SubscriptionHandler.h>
#include <msgs/ControlMsgs.h>
#include "ui/View.h"

class TerminalView : public View {
public:
    TerminalView(Node& node);

    virtual void create(RenderContext& context) override;
    virtual std::string title() override;
    virtual void render(RenderContext& context) override;

    virtual void handle_key(SDL_Keysym key, uint32_t type) override;

    void handle_terminal_override(void* usr, const TerminalOverrideMsg* msg);
    void handle_logging(void* usr, const LoggingMsg* msg);

private:
    std::vector<std::string> log_buffer_;
    std::string cmd_buffer_;
    std::unique_ptr<SubscriptionHandler> sub;

    Publisher* cmd_pub;
};

