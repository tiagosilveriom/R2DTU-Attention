#include "naoqi.h"

#include <stdlib.h>
#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wplacement-new="
#include <alcommon/albroker.h>
#include <alproxies/dcmproxy.h>
#include <alproxies/alaudioplayerproxy.h>
#include <alproxies/almemoryproxy.h>
#include <alproxies/alsystemproxy.h>
#include <alproxies/altexttospeechproxy.h>
#include <qi/anyobject.hpp>
#pragma GCC diagnostic pop


struct Context {
    boost::shared_ptr<AL::ALBroker> broker;
    AL::ALAudioPlayerProxy* audio_player;
    AL::ALSystemProxy* sys;
    AL::ALMemoryProxy* al_mem;
    AL::ALTextToSpeechProxy* tts;
    qi::SessionPtr session;
    qi::AnyObject tablet;

    AL::DCMProxy* dcm;
    AL::ALValue dcm_cmd;
};

static Context* ctx;

//System

bool naoqi_initialize() {

    //Allocate global storage
    auto mem = malloc(sizeof(Context));
    ctx = new(mem) Context();

    //Setup NaoQI API Proxies
    //These serializes all requests as XML (really?) and sends them local daemon.
    //Each call to a proxy therefore has to hit the network through kernel space and are therefore a bit expensive
    ctx->broker = AL::ALBroker::createBroker("LocalBroker", "0.0.0.0", 54000, "127.0.0.1", 9559);
    ctx->al_mem = new AL::ALMemoryProxy();
    ctx->audio_player = new AL::ALAudioPlayerProxy();
    ctx->sys = new AL::ALSystemProxy();
    ctx->tts = new AL::ALTextToSpeechProxy();
    ctx->dcm = new AL::DCMProxy(ctx->broker);
    ctx->dcm_cmd.arraySetSize(3);
    ctx->dcm_cmd[2].arrayReserve(255);

    //Setup the tablet
    ctx->session = qi::makeSession();
    ctx->session->connect("tcp://127.0.0.1:9559");
    ctx->tablet = ctx->session->service("ALTabletService");
    ctx->tablet.call<void>("resetTablet");

    return true;
}

void naoqi_shutdown() {
    ctx->~Context();
    free(ctx);
}

void naoqi_shutdown_robot() {
    ctx->sys->shutdown();
}

void naoqi_reboot_robot() {
    ctx->sys->reboot();
}

//Audio

void naoqi_tts_say(const char* sentence) {
    ctx->tts->say(std::string(sentence));
}

void naoqi_tts_say_to_file(const char* sentence, const char* filename) {
    ctx->tts->sayToFile(std::string(sentence), std::string(filename));
}

void naoqi_tts_set_language(const char* language) {
    ctx->tts->setLanguage(std::string(language));
}

float naoqi_tts_get_speed() {
    return ctx->tts->getParameter("speed");
}

void naoqi_tts_set_speed(float ratio) {
    ctx->tts->setParameter("speed", 100 * ratio);
}

float naoqi_tts_get_pitch() {
    return ctx->tts->getParameter("pitch");
}

void naoqi_tts_set_pitch(float ratio) {
    ctx->tts->setParameter("pitch", 100 * ratio);
}

float naoqi_tts_get_volume() {
    return ctx->tts->getParameter("volume");
}

void naoqi_tts_set_volume(float gain) {
    ctx->tts->setParameter("volume", 100 * gain);
}

void naoqi_tts_interrupt_speech() {
    ctx->tts->stopAll();
}

naoqi_audio_handle_t naoqi_audio_load_file(const char* file_name) {
    return ctx->audio_player->loadFile(std::string(file_name));
}

void naoqi_audio_play_handle(naoqi_audio_handle_t handle, float volume, float pan) {
    ctx->audio_player->play(handle, volume, pan);
}

void naoqi_audio_stop_all() {
    ctx->audio_player->stopAll();
}


//DCM

int32_t naoqi_dcm_get_timestamp() {
    return ctx->dcm->getTime(0);
}

const std::string dcm_actuator_keys[20];
const std::string dcm_actuator_stiffness_keys[20];

const std::string dcm_update_types[4] = {"Merge", "ClearAll", "ClearAfter", "ClearBefore"};

void naoqi_dcm_send_command(uint8_t actuator, uint8_t update_type, bool is_stiffness, uint8_t num_points, float* point_values, int32_t* point_timestamps) {
    if (is_stiffness) {
        ctx->dcm_cmd[0] = dcm_actuator_keys[actuator];
    } else {
        ctx->dcm_cmd[0] = dcm_actuator_stiffness_keys[actuator];
    }
    ctx->dcm_cmd[1] = dcm_update_types[update_type];
    ctx->dcm_cmd[2].arraySetSize(num_points);
    for (uint32_t i = 0; i < num_points; ++i) {
        ctx->dcm_cmd[2][i].arraySetSize(2);
        ctx->dcm_cmd[2][i][0] = point_values[i];
        ctx->dcm_cmd[2][i][1] = point_timestamps[i];
    }
    ctx->dcm->set(ctx->dcm_cmd);
}
