#pragma once
#include <common.hpp>
#include <SDL3/SDL_audio.h>
#define MAX_AUDIOFILENAME 48
struct audiodevice_t
{
    SDL_AudioDeviceID m_deviceID;
    SDL_AudioSpec m_spec;
    audiodevice_t() { SDL_zero(m_spec); }
};

struct audiodata_t
{
    char m_name[MAX_AUDIOFILENAME] = {"invalidaudio"};
    uint8_t* m_buf;
    uint32_t m_len;
    SDL_AudioSpec m_spec;
    uint8_t m_volume;
    uint64_t m_duration_ms;
    audiodata_t() :  m_buf(NULL), m_len(0), m_volume(128), m_duration_ms(0) { SDL_zero(m_spec); }
    ~audiodata_t() { if(m_buf != NULL) SDL_free(m_buf); }
};


struct audiostream_t
{
    SDL_AudioStream* stream = NULL;
    bool in_use = false;
    uint64_t duration = 0; //ms, not IEngineTime
    uint64_t time_end = 0;
    audiodata_t* src = nullptr;
    audiostream_t() : stream(NULL), in_use(false), duration(0), time_end(0), src(nullptr) {}

    void use_for(uint64_t cur_time, uint64_t dur, audiodata_t* ad) { duration = dur; time_end = cur_time + dur; in_use = true;  src = ad;}
    void reset() { duration =  time_end = 0; src = nullptr; in_use = false; }
};

