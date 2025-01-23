#include <stdlib.h>
#include <string>
#include <thread>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wplacement-new="
#include <alcommon/albroker.h>
#include <alproxies/alaudioplayerproxy.h>
#include <alproxies/alaudiorecorderproxy.h>
#include <alproxies/albehaviormanagerproxy.h>
#include <alproxies/almemoryproxy.h>
#include <alproxies/almotionproxy.h>
#include <alproxies/alrobotpostureproxy.h>
#include <alproxies/alsystemproxy.h>
#include <alproxies/altexttospeechproxy.h>
#include <qi/anyobject.hpp>
#pragma GCC diagnostic pop

#include <core/Node.h>
#include <common/SubscriptionHandler.h>
#include <common/RingBuffer.h>
#include <msgs/NaoMsgs.h>
#include <msgs/AudioMsgs.h>
#include <msgs/ControlMsgs.h>

struct Context {

    Context() : tts_buffer(10) {}

    Node* node;
    boost::shared_ptr<AL::ALBroker> broker;
    AL::ALAudioPlayerProxy* audio_player;
    AL::ALBehaviorManagerProxy* behavior;
    AL::ALSystemProxy* sys;
    AL::ALMemoryProxy* al_mem;
    AL::ALMotionProxy* motion;
    AL::ALRobotPostureProxy* posture;
    AL::ALTextToSpeechProxy* tts;
    AL::ALAudioRecorderProxy* audio_recorder;
    qi::SessionPtr session;
    qi::AnyObject tablet;

    std::thread tts_worker;


    std::unordered_map<std::string, int32_t> audio_handles;

    RingBuffer<std::string> tts_buffer;

    std::unique_ptr<SubscriptionHandler> sub;

    Publisher* tts_network_pub;

    bool tts_over_network;
};

const std::string actuator_keys[20] = {
        "HeadPitch",
        "HeadYaw",
        "HipRoll",
        "HipPitch",
        "KneePitch",
        "LElbowRoll",
        "LElbowYaw",
        "LHand",
        "LShoulderPitch",
        "LShoulderRoll",
        "LWristYaw",
        "RElbowRoll",
        "RElbowYaw",
        "RHand",
        "RShoulderPitch",
        "RShoulderRoll",
        "RWristYaw",
        "WheelFL",
        "WheelFR",
        "WheelB",
};

const float MAX_SPEED_TYPE_1 = 5.9f;
const float MAX_SPEED_TYPE_2 = 5.0f; //7.5 according to datasheet, but seems slower in practice
const float MAX_SPEED_TYPE_3A = 15.1f;
const float MAX_SPEED_TYPE_3B = 21.1f;
const float MAX_SPEED_TYPE_4 = 4.2f;
const float MAX_SPEED_TYPE_5 = 5.9f;
const float MAX_SPEED_TYPE_6 = 25.6f;

const float max_speeds[20] = {
        MAX_SPEED_TYPE_2, //HeadPitch,
        MAX_SPEED_TYPE_1, //"HeadYaw",
        MAX_SPEED_TYPE_4, //"HipRoll",
        MAX_SPEED_TYPE_5, //"HipPitch",
        MAX_SPEED_TYPE_5, //"KneePitch",
        MAX_SPEED_TYPE_2, //"LElbowRoll",
        MAX_SPEED_TYPE_1, //"LElbowYaw",
        MAX_SPEED_TYPE_3B, //"LHand",
        MAX_SPEED_TYPE_1, //"LShoulderPitch",
        MAX_SPEED_TYPE_2, //"LShoulderRoll",
        MAX_SPEED_TYPE_3A, //"LWristYaw",
        MAX_SPEED_TYPE_2, //"RElbowRoll",
        MAX_SPEED_TYPE_1, //"RElbowYaw",
        MAX_SPEED_TYPE_3B, //"RHand",
        MAX_SPEED_TYPE_1, //"RShoulderPitch",
        MAX_SPEED_TYPE_2, //"RShoulderRoll",
        MAX_SPEED_TYPE_3A, //"RWristYaw",
        MAX_SPEED_TYPE_6, //"WheelFL",
        MAX_SPEED_TYPE_6, //"WheelFR",
        MAX_SPEED_TYPE_6, //"WheelB",
};

static void move_to(void* usr, const MoveToMsg* msg) {
    auto ctx = (Context*) usr;
    ctx->motion->moveTo(msg->forward, msg->left, msg->theta);
}

struct ActCmd {
    int32_t timepoint;
    float value;
};

struct ActCmds {
    uint8_t act;
    uint8_t idx;
    std::vector<ActCmd> cmds;
};

static void trajectory_cmd(void* usr, const TrajectoryCmdMsg* msg) {

    auto ctx = (Context *) usr;

    auto num_timepoints = msg->num_timepoints;
    auto timestamps_ptr = Messages::access_blob(&msg->timestamps);
    auto num_values_ptr = Messages::access_blob(&msg->num_values);
    auto actuators_ptr = Messages::access_blob(&msg->actuators);
    auto values_ptr = Messages::access_blob(&msg->values);

    auto timestamps = (int32_t*) timestamps_ptr.first;
    auto num_values = (uint8_t*) num_values_ptr.first;
    auto actuators = (uint8_t*) actuators_ptr.first;
    auto values = (float*) values_ptr.first;

    printf("Receive\n");
    for (auto j = 0u; j < num_timepoints; ++j ) {
        for (auto k = 0u; k < num_values[j]; ++k) {
            printf("%u\n", actuators[j + k]);
        }
    }


    std::vector<ActCmds> actuator_cmds;

    auto p = 0u;
    for (auto i = 0; i < num_timepoints; ++i) {
        for (auto j = 0u; j < num_values[i]; ++j) {
            auto act = actuators[p + j];
            auto act_value = values[p + j];
            bool found_act = false;
            for (auto& a : actuator_cmds) {
                if (a.act == act) {
                    a.cmds.push_back({i, act_value});
                    found_act = true;
                    break;
                }
            }
            if (!found_act) {
                std::vector<ActCmd> cmds;
                cmds.push_back({i, act_value});
                actuator_cmds.push_back({act, 0, cmds});
            }
        }
        p += num_values[i];
    }

    auto current_time = 0u;

    AL::ALValue actuator_names;
    AL::ALValue actuator_values;
    std::vector<float> actuator_speeds;

    for (auto i = 0; i < num_timepoints; ++i) {
        actuator_names.arraySetSize(num_values[i]);
        actuator_values.arraySetSize(num_values[i]);
        actuator_speeds.resize(num_values[i], 0.0f);
        auto timepoint = timestamps[i];

        auto p = 0u;
        for (auto j = 0u; j < actuator_cmds.size(); ++j) {
            auto& act = actuator_cmds[j];
            if (act.idx == act.cmds.size()) continue;

            auto cmd = act.cmds[act.idx];
            if (cmd.timepoint == i) {
                auto delta_time = timepoint - current_time;
                auto act_name = actuator_keys[act.act];
                auto current_angle = ctx->motion->getAngles(act_name, true)[0];
                auto delta_angle = cmd.value - current_angle;
                printf("Act: %s\n", act_name.c_str());

                auto required_speed = std::fabs(delta_angle / (0.001f * delta_time * max_speeds[act.act]));

                auto speed = std::max(std::min(required_speed, 1.0f), 0.2f);


                actuator_names[p] = act_name;
                actuator_values[p] = cmd.value;
                actuator_speeds[p] = speed;
                p++;

                act.idx++;
            }
        }
        ctx->motion->setAngles(actuator_names, actuator_values, actuator_speeds);
        if (i < num_timepoints - 1) {
            usleep((timestamps[i] - current_time) * 1000);
        }
        current_time = timestamps[i];
    }
}

static void joint_cmd(void* usr, const JointCmdMsg* msg) {
    auto ctx = (Context *) usr;

    auto actuators_ptr = Messages::access_blob(&msg->actuators);
    auto values_ptr = Messages::access_blob(&msg->values);

    auto actuators = (uint8_t*) actuators_ptr.first;
    auto values = (float*) values_ptr.first;

    AL::ALValue actuator_names;
    AL::ALValue actuator_values;
    actuator_names.arraySetSize(msg->num_values);
    actuator_values.arraySetSize(msg->num_values);

    for (int i = 0; i < msg->num_values; ++i) {
        actuator_names[i] = actuator_keys[actuators[i]];
        actuator_values[i] = values[i];
    }

    ctx->motion->setAngles(actuator_names, actuator_values, msg->speed);
}

static void tablet_cmd(void* usr, const TabletCmdMsg* msg) {
    auto ctx = (Context*) usr;
    switch (msg->type) {
        case TabletCmdType::Sleep: {
            ctx->tablet.call<void>("goToSleep");
        }; break;
        case TabletCmdType::WakeUp: {
            ctx->tablet.call<void>("wakeUp");
        }; break;
        case TabletCmdType::Reset: {
            ctx->tablet.call<void>("resetTablet");
            ctx->tablet.call<bool>("loadApplication", "j-tablet-browser");
        }; break;
        case TabletCmdType::LoadUrl: {
            auto url = Messages::read_string(&msg->arg);
            ctx->tablet.call<bool>("loadUrl", url);
        }; break;
        case TabletCmdType::ShowImg: {
            auto img_url = Messages::read_string(&msg->arg);
            ctx->tablet.call<bool>("showImageNoCache", img_url);
        }; break;
    }
}

static void tts_runner(void* usr) {
    auto ctx = (Context*) usr;
    while (true) {
        auto sentence = ctx->tts_buffer.take();
        ctx->tts->say(sentence);
    }
}

static void tts_say(void* usr, const TTSSayMsg* msg) {
    auto ctx = (Context*) usr;
    auto sentence = Messages::read_string(&msg->text);
    if (ctx->tts_over_network) {
	std::string ram_file = "/dev/shm/tts_output.wav";
        ctx->tts->sayToFile(sentence, ram_file);
	FILE* fd = fopen(ram_file.c_str(), "rb");
	fseek(fd, 0, SEEK_END);
	long size = ftell(fd);
	rewind(fd);
	auto wav_blob = (uint8_t*) malloc(size);
	assert(fread(wav_blob, 1, size, fd) == size);
	fclose(fd);

	MessageBuilder<WavMsg> msg;

	msg.write_blob(&msg->payload, wav_blob, size);

	ctx->tts_network_pub->publish(msg);
	printf("Msg sent\n");
    } else {
        ctx->tts_buffer.put(sentence);
    }
}

static void tts_say_to_file(void* usr, const TTSSayToFileMsg* msg) {
    auto ctx = (Context*) usr;
    auto sentence = Messages::read_string(&msg->text);
    auto filename = Messages::read_string(&msg->file);
    ctx->tts->sayToFile(sentence, filename);
}

static void tts_settings(void* usr, const TTSSettingsMsg* msg) {
    auto ctx = (Context*) usr;
    if (msg->lang == TTSLang::Danish) {
        ctx->tts->setLanguage("Danish");
    } else {
        ctx->tts->setLanguage("English");
    }
    ctx->tts->setParameter("pitch", msg->pitch * 100.0f);
    //ctx->tts->setParameter("volume", msg->volume * 100.0f);
    ctx->tts->setParameter("speed", msg->speed * 100.0f);
    ctx->tts_over_network = msg->over_network;
}

static void toggle_breathing(void* usr, const ToggleBreathingMsg* msg) {
    auto ctx = (Context*) usr;
    printf("Toggle %u\n", msg->enabled);
    ctx->motion->setBreathEnabled("Arms", msg->enabled);
}

static void audio_play(void* usr, const AudioPlayMsg* msg) {
    /*auto ctx = (Context*) usr;
    auto filename = Messages::read_string(&msg->filename);
    if (msg->do_not_cache) {
        ctx->audio_player->playFile(filename, msg->volume, msg->pan);
    } else {
        auto search = ctx->audio_handles.find(filename);
        int32_t handle;
        if (search != ctx->audio_handles.end()) {
            handle = search->second;
        } else {
            handle = ctx->audio_player->loadFile(filename);
            ctx->audio_handles.insert(handle);
        }
        ctx->audio_player->play(handle, msg->volume, msg->pan);
    }*/
}

static void audio_record(void* usr, const AudioRecordMsg* msg) {
    static bool is_recording = false;
    auto ctx = (Context*) usr;
    auto name = Messages::read_string(&msg->filename);
    printf("Recording cmd %s!\n", name.c_str());
    auto path = "/home/nao/" + name;
    if (!is_recording) {
        AL::ALValue channels;
        channels.arrayPush(0);
        channels.arrayPush(0);
        channels.arrayPush(1);
        channels.arrayPush(0);
        while (true) {
            try {
                ctx->audio_recorder->startMicrophonesRecording(path, "wav", 16000, channels);
                break;
            } catch (AL::ALError e) {
                ctx->audio_recorder->stopMicrophonesRecording();
            }
        }
        is_recording = true;
    } else {
        ctx->audio_recorder->stopMicrophonesRecording();
        is_recording = false;
    }
}

static void animation_cmd(void* usr, const AnimationCmdMsg* msg) {
    auto ctx = (Context*) usr;
    auto name = Messages::read_string(&msg->name);
    ctx->behavior->runBehavior(name);
}

void handle_gamepad_button(void* usr_ptr, const GamepadButtonMsg* msg) {
    auto ctx = (Context*) usr_ptr;

    printf("Button %u\n", msg->button_id);

    try {
        if (msg->button_id == GAMEPAD_A && msg->state) {
            ctx->behavior->startBehavior("animations/Novo/Talk_14");
        }

        if (msg->button_id == GAMEPAD_Y && msg->state) {
            ctx->behavior->startBehavior("animations/Stand/Gestures/BowShort_1");
        }

        static int handshake_state = 0;

        if (msg->button_id == GAMEPAD_B && msg->state) {
            if (handshake_state == 0) {
                AL::ALValue names = AL::ALValue::array("RElbowRoll", "RWristYaw", "RElbowYaw", "RHand", "RShoulderPitch", "RShoulderRoll");
                AL::ALValue angles = AL::ALValue::array(0.14, 1.7, 0.0, 1.0, -0.3, -0.1);
                ctx->motion->setAngles(names, angles, 0.3);
                handshake_state = 1;
                ctx->tts_buffer.put("Hello!");
            } else if (handshake_state == 1) {
                AL::ALValue names = "RHand";
                AL::ALValue angles = AL::ALValue::array(0.7, 0.7, 0.9);
                AL::ALValue times = AL::ALValue::array(0.5, 2.5, 3.0);
                ctx->motion->angleInterpolation(names, angles, times, true);
                handshake_state = 2;
                ctx->tts_buffer.put("My name is R2DTU!");
            } else if (handshake_state == 2) {
                AL::ALValue names = AL::ALValue::array("RElbowRoll", "RElbowYaw", "RHand", "RShoulderPitch", "RShoulderRoll", "RWristYaw");
                AL::ALValue angles = AL::ALValue::array(0.26, 0.87, 0.6, 1.56, -0.17, 0.0);
                ctx->motion->setAngles(names, angles, 0.5);
                handshake_state = 0;
                usleep(10000);
                ctx->behavior->startBehavior("animations/Stand/Gestures/Far_1");
                usleep(15000);
                ctx->tts_buffer.put("Welcome to Robo insight!");
            }
        }

        if (msg->button_id == GAMEPAD_X && msg->state) {
            ctx->posture->goToPosture("Stand", 0.3);
        }

        if (msg->button_id == GAMEPAD_CIRCLE && msg->state) {
            ctx->tts_buffer.put("Hello everyone!");
            ctx->behavior->startBehavior("animations/Novo/Talk_14");
            ctx->tts_buffer.put("And welcome to Robo insight!");
        }
    } catch (AL::ALError e) {

    }

}

void handle_gamepad_axis(void* usr_ptr, const GamepadAxisMsg* msg) {

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    auto ctx = (Context*) usr_ptr;
    if (msg->axis_id == 0) {
        double angle = (-msg->value / 29000.0) * M_PI_2;
        ctx->motion->setAngles("HeadYaw", angle, 0.1);
    }
    if (msg->axis_id == 1) {
        double angle = (-msg->value / 29000.0) * M_PI_2 - 0.2;
        ctx->motion->setAngles("HeadPitch", angle, 0.1);
    }

    if (msg->axis_id == 2) {
        z = (-msg->value / 29000.0);
        ctx->motion->move(x, y, z);
    }

    if (msg->axis_id == 3) {
        x = (-msg->value / 29000.0);
        ctx->motion->move(x, y, z);
    }
}


extern "C" {

Context* initialize(Node* node) {
    auto ctx = new Context();
    ctx->node = node;

    ctx->broker = AL::ALBroker::createBroker("LocalBroker", "0.0.0.0", 54000, "127.0.0.1", 9559);
    ctx->al_mem = new AL::ALMemoryProxy();
    ctx->audio_player = new AL::ALAudioPlayerProxy();
    ctx->behavior = new AL::ALBehaviorManagerProxy();
    ctx->sys = new AL::ALSystemProxy();
    ctx->tts = new AL::ALTextToSpeechProxy();
    ctx->motion = new AL::ALMotionProxy();
    ctx->posture = new AL::ALRobotPostureProxy();
    ctx->audio_recorder = new AL::ALAudioRecorderProxy();

    ctx->sub = std::make_unique<SubscriptionHandler>(node);

    ctx->sub->subscribe<MoveToMsg>("move_to_cmd", &move_to, ctx);
    ctx->sub->subscribe<TrajectoryCmdMsg>("trajectory_cmd", &trajectory_cmd, ctx);
    ctx->sub->subscribe<JointCmdMsg>("joint_cmd", &joint_cmd, ctx);
    ctx->sub->subscribe<TabletCmdMsg>("tablet_cmd", &tablet_cmd, ctx);
    ctx->sub->subscribe<TTSSettingsMsg>("tts_settings", &tts_settings, ctx);
    ctx->sub->subscribe<TTSSayMsg>("tts_say", &tts_say, ctx);
    ctx->sub->subscribe<TTSSayToFileMsg>("tts_say_to_file", &tts_say_to_file, ctx);
    ctx->sub->subscribe<AudioPlayMsg>("audio_play", &audio_play, ctx);
    ctx->sub->subscribe<AnimationCmdMsg>("animation_cmd", &animation_cmd, ctx);
    ctx->sub->subscribe<ToggleBreathingMsg>("toggle_breathing", &toggle_breathing, ctx);
    ctx->sub->subscribe<GamepadButtonMsg>("gamepad_button", handle_gamepad_button, ctx);
    ctx->sub->subscribe<GamepadAxisMsg>("gamepad_axis", handle_gamepad_axis, ctx);
    ctx->sub->subscribe<AudioRecordMsg>("audio_record", &audio_record, ctx);

    ctx->tts_network_pub = node->advertise<WavMsg>("tts_network");
    ctx->tts_over_network = false;

    ctx->tts_worker = std::thread([ctx]() {
        tts_runner(ctx);
    });

    usleep(500);
    ctx->motion->setAngles("HeadPitch", 0.0, 0.1);
    ctx->motion->setAngles("HeadYaw", 0.0, 0.1);
    ctx->motion->setAngles("HipPitch", -0.08, 0.1);
    ctx->motion->setAngles("KneePitch", -0.08, 0.1);

    return ctx;
}

void tick(Node* node, Context* ctx) {
    ctx->sub->tick();
}

void shutdown(Node* node, Context* ctx) {
    delete ctx;
}

void unload(Node* node, Context* ctx) {

}

void reload(Node* node, Context* ctx) {

}

} //extern "C"
