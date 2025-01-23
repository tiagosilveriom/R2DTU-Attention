//
// Created by dissing on 8/14/19.
//

#include <algorithm>
#include <random>

static std::vector<std::string> thinking_behaviors = {
        "animations/Stand/Gestures/Thinking_2",
        "animations/Stand/Gestures/Thinking_4",
        "animations/Stand/Gestures/Thinking_8",
};

static std::vector<std::string> listening_behaviors = {
        "animations/Stand/BodyTalk/Listening/Listening_1",
        "animations/Stand/BodyTalk/Listening/Listening_2",
        "animations/Stand/BodyTalk/Listening/Listening_3",
        "animations/Stand/BodyTalk/Listening/Listening_4",
        "animations/Stand/BodyTalk/Listening/Listening_5",
        "animations/Stand/BodyTalk/Listening/Listening_6",
        "animations/Stand/BodyTalk/Listening/Listening_7"
};

static std::vector<std::string> speaking_behaviors = {
        "animations/Stand/BodyTalk/Speaking/BodyTalk_1",
        "animations/Stand/BodyTalk/Speaking/BodyTalk_10",
        "animations/Stand/BodyTalk/Speaking/BodyTalk_11",
        "animations/Stand/BodyTalk/Speaking/BodyTalk_12",
        "animations/Stand/BodyTalk/Speaking/BodyTalk_13",
        "animations/Stand/BodyTalk/Speaking/BodyTalk_14",
        "animations/Stand/BodyTalk/Speaking/BodyTalk_15",
        "animations/Stand/BodyTalk/Speaking/BodyTalk_16",
        "animations/Stand/BodyTalk/Speaking/BodyTalk_2",
        "animations/Stand/BodyTalk/Speaking/BodyTalk_3",
        "animations/Stand/BodyTalk/Speaking/BodyTalk_4",
        "animations/Stand/BodyTalk/Speaking/BodyTalk_5",
        "animations/Stand/BodyTalk/Speaking/BodyTalk_6",
        "animations/Stand/BodyTalk/Speaking/BodyTalk_7",
        "animations/Stand/BodyTalk/Speaking/BodyTalk_8",
        "animations/Stand/BodyTalk/Speaking/BodyTalk_9"
};

static std::vector<std::string> waiting_behaviors = {
        "animations/Stand/Waiting/PlayHands_1",
        "animations/Stand/Waiting/PlayHands_2",
        "animations/Stand/Waiting/PlayHands_3",
        "animations/Stand/Waiting/ShowMuscles_1",
        "animations/Stand/Waiting/ShowMuscles_2",
        "animations/Stand/Waiting/ShowMuscles_3",
};

class Randomizer {
public:
    Randomizer() : rng({}) {}
    template <typename T>
    T& choose(std::vector<T>& v) {
        if (i == 0) {
            std::shuffle(v.begin(), v.end(), rng);
        }
        auto chosen = i;
        i = (i + 1) % v.size();
        return v[chosen];
    }
private:
    int i = 0;
    std::default_random_engine rng;
};

const std::string& get_random_waiting_animation() {
    static Randomizer r = Randomizer();
    return r.choose(waiting_behaviors);
}

const std::string& get_random_thinking_animation() {
    static Randomizer r = Randomizer();
    return r.choose(thinking_behaviors);
}

const std::string& get_random_speaking_animation() {
    static Randomizer r = Randomizer();
    return r.choose(speaking_behaviors);
}

const std::string& get_random_listening_animation() {
    static Randomizer r = Randomizer();
    return r.choose(listening_behaviors);
}
