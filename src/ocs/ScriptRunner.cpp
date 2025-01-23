#include "ScriptRunner.h"
#include "ui/Audio.h"
#include <json/json.hpp>

using json = nlohmann::json;

#include <msgs/ActuationMsgs.h>

void ScriptRunner::do_animation(std::string name) {
    MessageBuilder<AnimationActuationMsg> msg;
    msg->request.requester_id = 0;
    msg->request.request_id = next_request++;
    msg.write_string(&msg->name, name);
    animation_pub_->publish(msg);
}

void ScriptRunner::do_say(std::string sentence) {
    MessageBuilder<SayActuationMsg> msg;
    msg->request.requester_id = 0;
    msg->request.request_id = next_request++;
    msg.write_string(&msg->sentence, sentence);
    msg->failed_to_understand = false;
    say_pub_->publish(msg);
}

void ScriptRunner::handle_gamepad_button(void* ctx, const GamepadButtonMsg* msg) {
    if (msg->button_id == GAMEPAD_R2 && msg->state == 1) {
        if (current_item_ < items_.size()) {
            items_[current_item_]->execute();
            if (current_item_ + 1 < items_.size()) {
                current_item_++;
            }
        }
    }
    if (msg->button_id == GAMEPAD_PLUS && msg->state == 1) {
        if (current_item_ + 1 < items_.size()) {
            current_item_++;
        }
    }
    if (msg->button_id == GAMEPAD_MINUS && msg->state == 1) {
        if (current_item_ > 0) {
            current_item_--;
        }
    }
}

void ScriptRunner::add_item(std::string desc, std::string sentence, std::vector<std::unique_ptr<ScriptSubItem>> sub_items) {
    items_.push_back(std::make_unique<ScriptItem>(desc, sentence, std::move(sub_items)));
}

void ScriptRunner::load_script(std::string filename) {
    std::string content;
    auto fp = fopen(filename.c_str(), "r");
    if (!fp) {
        printf("Error! Failed to open script file: %s\n", filename.c_str());
        return;
    }
    fseek(fp, 0, SEEK_END);
    content.resize(ftell(fp));
    rewind(fp);
    if (fread(&content[0], 1, content.size(), fp) == 0) {
        printf("Error! Failed to read from script file: %s\n", filename.c_str());
        return;
    }
    fclose(fp);

    auto j = json::parse(content);

    for (auto& elem : j) {
        std::string desc = elem["short"];
        std::string note = elem["note"];
        std::vector<std::unique_ptr<ScriptSubItem>> sub_items;

        for (auto& sub : elem["items"]) {
            std::string type = sub[0];
            if (type == "play_wav") {
                std::string name = sub[1];
                sub_items.emplace_back(std::make_unique<PlayWavSubItem>(this, name));
            } else if (type == "animation") {
                std::string name = sub[1];
                sub_items.emplace_back(std::make_unique<AnimationSubItem>(this, name));
            } else if (type == "say") {
                std::string sentence = sub[1];
                sub_items.emplace_back(std::make_unique<SaySubItem>(this, sentence));
            }
        }

        add_item(desc, note, std::move(sub_items));
    }

}

ScriptRunner::ScriptRunner(Node &node)
        : state_(State::Ready), current_item_(0) {

    animation_pub_ = node.advertise<AnimationActuationMsg>("animation_actuation");
    say_pub_ = node.advertise<SayActuationMsg>("say_actuation");

    load_script("../resources/scripts/forsk.json");

    sub_ = new SubscriptionHandler(&node);

    using namespace std::placeholders;
    sub_->subscribe<GamepadButtonMsg>("gamepad_button", std::bind(&ScriptRunner::handle_gamepad_button, this, _1, _2), nullptr);
}

void ScriptRunner::tick() {
    sub_->tick();
}

std::unordered_map<ScriptRunner::State, std::string> state_names = {
        {ScriptRunner::State::Ready, "Ready"},
        {ScriptRunner::State::Waiting, "Waiting"},
        {ScriptRunner::State::Running, "Running"},
        {ScriptRunner::State::Done, "Done"},
};

std::string& ScriptRunner::state_to_string(ScriptRunner::State state) {
    return state_names[state];
}

