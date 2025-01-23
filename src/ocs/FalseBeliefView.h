#pragma once

#include <core/Node.h>
#include <common/SubscriptionHandler.h>
#include <msgs/ReasoningMsgs.h>
#include "ui/View.h"


class FalseBeliefView : public View {
public:
    FalseBeliefView(Node& node);

    virtual void create(RenderContext& context) override;
    virtual std::string title() override;
    virtual void render(RenderContext& context) override;


    void update_view_msg(void* ctx, const EpistemicStateViewMsg* msg);

    struct EpistemicStateView {
        std::string name;
        std::vector<char> containers;
        std::vector<char> movables_location;
        std::vector<char> movables_id;
    };

private:
    std::unique_ptr<SubscriptionHandler> sub;

    std::vector<EpistemicStateView> views;
    std::unordered_map<std::string, size_t> view_index;
};
