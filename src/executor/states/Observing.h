#pragma once

#include <reasoning/EpistemicReasoning.h>

class Observing : public State {
public:

    Observing(Node *node) : epistemic(*node), sub(std::make_unique<SubscriptionHandler>(node)) {
        using namespace std::placeholders;
        sub->subscribe<TerminalCmdMsg>("terminal_cmd", std::bind(&Observing::terminal_cmd, this, std::placeholders::_1,
                                                                 std::placeholders::_2), nullptr);
        //speech_triggers.push_back(std::bind(&Observing::fetch_response_trigger, this, _1));
        speech_triggers.push_back(std::bind(&Observing::goal_statement_trigger, this, _1));
        speech_triggers.push_back(std::bind(&Observing::why_speech_trigger, this, _1));
        speech_triggers.push_back(std::bind(&Observing::need_speech_trigger, this, _1));
        speech_triggers.push_back(std::bind(&Observing::epistemic_speech_trigger, this, _1));
    }

    void terminal_cmd(void *usr, const TerminalCmdMsg *msg);

    void enter(Robot& r) override ;
    bool tick(Robot& r) override ;
    void leave(Robot& r) override;

private:

    using SpeechTrigger = std::function<bool(const std::string&)>;

    void intention_trigger(WorldObject* agent, WorldObject* moveable);
    void intention_trigger_complementary(WorldObject* agent, WorldObject* moveable);

    Option<WorldEvent> object_new_transformer(World& world, WorldEvent event);
    Option<WorldEvent> object_appear_transformer(World& world, WorldEvent event);
    Option<WorldEvent> object_disappear_transformer(World& world, WorldEvent event);

    void inform_about_false_belief_box(const std::string& agent, const std::string& box);
    void inform_about_false_belief_object(const std::string& agent, const std::string& obj);

    void inform_about_false_belief_cube(const std::string& agent, const std::string& cube);

    void fbt2_push_trigger(World& world);

    bool epistemic_speech_trigger(const std::string& s);
    bool fetch_response_trigger(const std::string& s);
    bool goal_statement_trigger(const std::string& s);
    bool need_speech_trigger(const std::string& s);
    bool why_speech_trigger(const std::string& s);

    void start_listening();
    void stop_listening();

    EpistemicReasoning epistemic;
    std::unordered_map<std::string, std::string> current_container;
    std::unique_ptr<Sequence> current_sequence;
    std::unique_ptr<SubscriptionHandler> sub;

    std::vector<SpeechTrigger> speech_triggers;

    std::string explain_sentence;
    uint32_t expect_fetch_response_box_id = 0;

    uint32_t counter = 0;

    struct PersonInfo {
        bool turned_back;
    };
    std::unordered_map<std::string, PersonInfo> person_info;
    Robot* robot;
    bool recording = false;
};


std::string get_matching_affordances(const std::string& obj);
