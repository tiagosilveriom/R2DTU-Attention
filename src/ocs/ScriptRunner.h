#pragma once

#include <functional>
#include <core/Node.h>
#include <common/SubscriptionHandler.h>

#include <msgs/ControlMsgs.h>
#include <msgs/ActuationMsgs.h>
#include "ui/Audio.h"

class ScriptSubItem {
public:
    virtual void execute() = 0;
};

class ScriptItem {
public:
    ScriptItem(std::string short_desc, std::string sentence, std::vector<std::unique_ptr<ScriptSubItem>> sub_items)
    : short_desc_(short_desc + "..."), sentence_(sentence), sub_items_(std::move(sub_items)) {
        int last_space = -1;
        int last_cut = 0;
        for (int i = 0; i < (int) sentence.size(); ++i) {
            if (sentence[i] == ' ') last_space = i;

            if ((i - last_cut) > 40) {
                if (last_space == -1) last_space = i; //If we have not found any spaces on this line, we just cut right here
                long_desc_.push_back(sentence_.substr(last_cut, last_space - last_cut));
                last_cut = last_space;
                i = last_space;
                last_space = -1;
            }
        }
        long_desc_.push_back(sentence_.substr(last_cut, sentence.size() - last_cut));
    }

    const std::string& short_desc() const {
        return short_desc_;
    }

    const std::vector<std::string>& long_desc() const {
        return long_desc_;
    }

    void execute() {
        for (auto& item : sub_items_) {
            item->execute();
        }
    }

private:

    std::string short_desc_;
    std::string sentence_;
    std::vector<std::string> long_desc_;
    std::vector<std::unique_ptr<ScriptSubItem>> sub_items_;
};

class ScriptRunner {
public:

    enum class State {
        Ready,
        Waiting,
        Running,
        Done
    };

    explicit ScriptRunner(Node& node);

    int current_item_id() const {
        return current_item_;
    }

    int total_items() const {
        return (int) items_.size();
    }

    const ScriptItem& get_item(int id) {
        return *items_[id].get();
    }

    State current_state() const {
        return state_;
    }

    static std::string& state_to_string(State state);

    int running_time() {
        return 168;
    }

    void load_script(std::string filename);

    void tick();

    void do_animation(std::string name);

    void do_say(std::string sentence);

    void handle_gamepad_button(void* ctx, const GamepadButtonMsg* msg);

private:

    void add_item(std::string desc, std::string sentence, std::vector<std::unique_ptr<ScriptSubItem>>);

    State state_;
    uint32_t current_item_;
    std::vector<std::unique_ptr<ScriptItem>> items_;

    uint32_t next_request = 0;

    Publisher* animation_pub_;
    Publisher* say_pub_;

    std::vector<uint32_t > audio_;

    int counter = 0;

    SubscriptionHandler* sub_;

};

class PlayWavSubItem : public ScriptSubItem {
public:
    PlayWavSubItem(ScriptRunner* runner, std::string& audio_name) {
        handle_ = Audio::load_wav(audio_name);
    }

    void execute() override {
        Audio::play_wav(handle_);
    }
private:
    uint32_t handle_;
};

class SaySubItem : public ScriptSubItem {
public:
    SaySubItem(ScriptRunner* runner, std::string& sentence)
        : runner_(runner), sentence_(sentence) {}

    void execute() override {
        runner_->do_say(sentence_);
    }
private:
    ScriptRunner* runner_;
    std::string sentence_;
};

class AnimationSubItem : public ScriptSubItem {
public:
    AnimationSubItem(ScriptRunner* runner, std::string& animation_name)
        : runner_(runner), animation_name_(animation_name) {}

    void execute() override {
        printf("Run %s\n", animation_name_.c_str());
        runner_->do_animation(animation_name_);
    }
private:
    ScriptRunner* runner_;
    std::string animation_name_;
};
