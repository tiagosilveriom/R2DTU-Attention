#pragma once

#include <string>
#include <vector>
#include <SDL2/SDL.h>

using AudioHandle = uint32_t;

class Audio {
public:
    static bool setup();
    static void reset();

    static AudioHandle load_wav(std::string path);
    static void sound_check();

    static void play_wav(AudioHandle handle);
    static void play_raw(uint8_t* buffer, uint32_t length);

    static void shutdown();

private:
    static SDL_AudioDeviceID device_;
    static SDL_AudioSpec spec_;
    static std::vector<std::pair<uint8_t*, uint32_t>> waves_;
};

