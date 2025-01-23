//
// Created by dissing on 2/2/19.
//

#include "Audio.h"
#include <SDL2/SDL.h>


std::vector<std::pair<uint8_t*, uint32_t>> Audio::waves_;
SDL_AudioSpec Audio::spec_;
SDL_AudioDeviceID Audio::device_;
AudioHandle sound_check_wav;

bool Audio::setup() {
    spec_.freq = 22050;
    spec_.channels = 1;
    spec_.samples = 4096;
    spec_.format = AUDIO_S16;
    spec_.callback = nullptr;
    device_ = SDL_OpenAudioDevice(NULL, 0, &spec_, NULL, 0);
    sound_check_wav = load_wav("../resources/sound_check.wav");
    return false;
}

void Audio::reset() {
    SDL_CloseAudioDevice(device_);
    device_ = SDL_OpenAudioDevice(NULL, 0, &spec_, NULL, 0);
}

void Audio::sound_check() {
    play_wav(sound_check_wav);
}

AudioHandle Audio::load_wav(std::string path) {
    uint32_t len;
    uint8_t* buffer;
    SDL_AudioSpec spec;
    SDL_LoadWAV(path.c_str(), &spec, &buffer, &len);
    waves_.push_back({buffer, len});
    return waves_.size() - 1;
}

void Audio::play_wav(AudioHandle handle) {
    SDL_QueueAudio(device_, waves_[handle].first, waves_[handle].second);
    SDL_PauseAudioDevice(device_, 0);
}

void Audio::play_raw(uint8_t* buffer, uint32_t length) {
    const char* shm_path = "/dev/shm/tts_output.wav";
    FILE* fp = fopen(shm_path, "wb");
    fwrite(buffer, 1, length, fp);
    fclose(fp);

    SDL_AudioSpec spec;
    SDL_LoadWAV(shm_path, &spec, &buffer, &length);

    /*uint8_t* fmt_chunk = &buffer[12];
    uint32_t* chunk_size = (uint32_t*) &fmt_chunk[4];
    size_t header_size = 12 + 8 + *chunk_size;
    uint8_t* data_start = &fmt_chunk[header_size];*/
    SDL_QueueAudio(device_, buffer, length);
    SDL_PauseAudioDevice(device_, 0);
    SDL_FreeWAV(buffer);
}

void Audio::shutdown() {
    for (auto& w : waves_) {
        SDL_FreeWAV(w.first);
    }
}
