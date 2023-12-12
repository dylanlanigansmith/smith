#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>
#include <logger/logger.hpp>
#include <queue>
#include <types/CTime.hpp>

#include "soundcommand.hpp"

#define SMITH_AUDIOFMT SDL_AUDIO_S16
#define SMITH_AUDIOSTREAMS 18 //hack

//not an interface but it might have one for a queue maybe.. this is the seperate thread for it

//https://wiki.libsdl.org/SDL3/SDL_AudioSpec

struct audiodevice_t
{
    SDL_AudioDeviceID m_deviceID;
    SDL_AudioSpec m_spec;
    audiodevice_t() { SDL_zero(m_spec); }
};

struct audiodata_t
{
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
    audiostream_t() : stream(NULL), in_use(false), duration(0), time_end(0) {}
    void use_for(uint64_t cur_time, uint64_t dur) { duration = dur; time_end = cur_time + dur; in_use = true; }
    void reset() { duration =  time_end = 0; in_use = false; }
};



class  /*LCD*/CSoundSystem : public CLogger
{
public:
    CSoundSystem() : CLogger(this), m_iNumStreams(SMITH_AUDIOSTREAMS) {}
    bool Init(int plat);
    void Shutdown();

    bool PlaySound(const std::string& name, float m_flVolume = 1.f, bool m_bLoop = false);

private:
    int Loop(void* data);
    int GetAudioDriver(bool list = false);
    void SetupAudioDevice();
   
    void LoadAudioFile(const std::string& name, uint8_t format = 0);
    audiodata_t* GetAudioByName(const std::string& name);

    void LogAudioData(const std::string& name);
    void LogAudioDevice();

    Time_t GetSoundDuration(audiodata_t* audio);
private:
    SoundQueue m_cmdQueue;
    float m_flVolumeMaster;

    std::string m_szAudioResourcePath;
    const size_t m_iNumStreams;


    std::unordered_map<std::string, audiodata_t*> soundboard;
    audiodevice_t m_device;
    SDL_Thread* m_mainThread;
    bool m_bVerbose;
    bool m_bShouldQuit;

    
};