#include "FalseBeliefView.h"
#include <msgs/ReasoningMsgs.h>

static const Vec2i BLOCK_OFFSETS[] = {
        {160,0}, {0, 240}, {320, 240}
};

void FalseBeliefView::update_view_msg(void* ctx, const EpistemicStateViewMsg* msg) {
    auto name = Messages::read_string(&msg->name);
    auto search = view_index.find(name);

    EpistemicStateView view;
    view.name = name;


    for (auto i = 0u; i < EpistemicStateViewMsg::MAX_CONTAINERS; ++i) {
        if (msg->containers[i] == '\0') break;
        view.containers.push_back(msg->containers[i]);
    }
    for (auto i = 0u; i < EpistemicStateViewMsg::MAX_MOVABLES; ++i) {
        if (msg->movables_id[i] == '\0') break;
        view.movables_id.push_back(msg->movables_id[i]);
        view.movables_location.push_back(msg->movables_location[i]);
    }

    if (search != view_index.end()) {
        views[search->second] = view;
    } else {
        view_index.insert({name, views.size()});
        views.push_back(view);
    }
}

FalseBeliefView::FalseBeliefView(Node& node)
    : View("Epistemic") {
    sub = std::make_unique<SubscriptionHandler>(&node);
    sub->subscribe<EpistemicStateViewMsg>("epistemic_view", std::bind(&FalseBeliefView::update_view_msg, this, std::placeholders::_1, std::placeholders::_2), nullptr);
}

void FalseBeliefView::create(RenderContext &context) {

}

std::string FalseBeliefView::title() {
    return "Epistemic Visualization";
}

Color movable_color_lookup(char id) {
    switch (id) {
        case '1' : 
        case 'a' : return Color::bright_red();
        case '2' : 
        case 'k' : return Color::bright_green();
        case '3' : 
        case 'b' : return Color::bright_blue();
        case 'c' : return Color::red();
        default: return Color::black();
    }
}

std::string movable_object_name(char id) {
    switch (id) {
        case 'b' : return "bottle";
        case 'k' : return "knife";
        case 'a' : return "apple";
        case 'c' : return "cup";
    }
}

void render_belief_block(RenderContext& context, Vec2i off, FalseBeliefView::EpistemicStateView& view) {

    int s = 16;

    context.fill_box(off, {320,240}, Color::white());
    context.draw_box(off, {320,240}, Color::background(), 2);

    context.draw_text(view.name, off + Vec2i{160, 40}, TextAlign::CENTER, 32, Color::black());

    for (auto i = 0u; i < view.containers.size(); ++i) {
        auto container = view.containers[i];

        int t = 5*s + off.y;
        int b = t+5*s;
        int l = (2.5+i*4)*s + off.x;
        int r = l+3*s;

        context.draw_line({l,t}, {l,b}, Color::black(), 3);
        context.draw_line({l,b}, {r,b}, Color::black(), 3);
        context.draw_line({r,b}, {r,t}, Color::black(), 3);
        context.draw_text(std::string(1, container), Vec2i{(int) (l+s*1.5), (int)(b+s*1.5)}, TextAlign::CENTER, 32, Color::black());

        for (auto j = 0u; j < view.movables_id.size(); ++j) {
            auto mov_id = view.movables_id[j];

            auto location = view.movables_location[j];
            if (location == container) {
                switch(mov_id)
                {
                    //markers
                    case '1':
                    case '2':
                    case '3': context.fill_box({l + s, (int) (t + (j+1) * s)}, {s,s}, movable_color_lookup(mov_id));
                              context.draw_box({l + s, (int) (t + (j+1) * s)}, {s,s}, Color::black(), 3);
                              break;
                    //objects          
                    default:    context.fill_box({l + s, (int) (t + (j+1) * s)}, {s,s}, movable_color_lookup(mov_id));
                                context.draw_box({l + s, (int) (t + (j+1) * s)}, {s,s}, Color::black(), 3);
                                    // Draw the text inside the box
                                context.draw_text(
                                movable_object_name(mov_id), // Convert mov_id to a string
                                Vec2i{l+s + s / 2, (int)(t + (j + 1) * s) + s/2}, // Center the text inside the box
                                TextAlign::CENTER, 
                                16, // Font size
                                Color::bright_green()
                                );
                                
                                break;
                }
            }
        }
    }

    for (auto i = 0u; i < view.movables_id.size(); ++i) {
        auto mov_id = view.movables_id[i];
        if (mov_id == '\0') break;
        auto location = view.movables_location[i];

        if (location == ' ') {
            int t = 12.5*s + off.y;
            int l = (8+i*3)*s + off.x; 

            switch(mov_id)
            {
                //markers
                case '1':
                case '2':
                case '3':   context.fill_box({l,t}, {s,s}, movable_color_lookup(mov_id));
                            context.draw_box({l,t}, {s,s}, Color::black(), 3);
                            break;
                //objects          
                default:    context.fill_box({l,t}, {s,s}, movable_color_lookup(mov_id));
                            context.draw_box({l, t}, {s, s}, Color::black(), 3);
                            // Draw the text inside the box
                            context.draw_text(
                                movable_object_name(mov_id), 
                                Vec2i{l + s / 2, t + s + 10}, // Center text in the box
                                TextAlign::CENTER, 
                                16, // Font size
                                Color::black()
                            );
            }
        }
    }
}

void FalseBeliefView::render(RenderContext& context) {
    sub->tick();
    for (auto i = 0u; i < views.size() && i < 3; ++i) {
        render_belief_block(context, BLOCK_OFFSETS[i], views[i]);
    }
}