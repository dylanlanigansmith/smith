#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>
#include <logger/logger.hpp>
//not an interface but it might have one for a queue maybe.. this is the seperate thread for it

//https://wiki.libsdl.org/SDL3/SDL_AudioSpec

struct audiodevice_t
{
    SDL_AudioDeviceID m_deviceID;
    SDL_AudioSpec m_spec;
    SDL_AudioStream* m_stream;
    audiodevice_t() { SDL_zero(m_spec); }
};

struct audiodata_t
{
    uint8_t* m_buf;
    uint32_t m_len;
    SDL_AudioSpec m_spec;
    uint8_t m_volume;
    audiodata_t() :  m_buf(NULL), m_len(0), m_volume(128) { SDL_zero(m_spec); }
    ~audiodata_t() { if(m_buf != NULL) SDL_free(m_buf); }
};

typedef std::pair<std::string, audiodata_t> AudioResource;


class  /*LCD*/CSoundSystem : public CLogger
{
public:
    CSoundSystem() : CLogger(this) {}
    bool Init(int plat);
    void Shutdown();
private:
    int Loop(void* data);
    int GetAudioDriver(bool list = false);
    void SetupAudioDevice();
    void AudioCallback(void* userdata, uint8_t* stream, int len);
    void LoadAudioFile(const std::string& name, uint8_t format = 0);
    audiodata_t* GetAudioByName(const std::string& name);

    void LogAudioData(const std::string& name);
    void LogAudioDevice();
private:
    std::string m_szAudioResourcePath;

    std::unordered_map<std::string, audiodata_t*> soundboard;
    audiodevice_t m_device;
    SDL_Thread* m_mainThread;
    bool m_bVerbose;
    bool m_bShouldQuit;
};