#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>
#include <logger/logger.hpp>
#include <queue>
#include <types/CTime.hpp>
#include <types/Vector.hpp>
#include "soundcommand.hpp"
#include "audio_types.hpp"
#include "streamgroup.hpp"
#define SMITH_AUDIOFMT SDL_AUDIO_S16
#define SMITH_AUDIOSTREAMS 18 //hack

//not an interface but it might have one for a queue maybe.. this is the seperate thread for it

//https://wiki.libsdl.org/SDL3/SDL_AudioSpec




class  /*LCD*/CSoundSystem : public CLogger
{
    friend class CEditor;
public:
    CSoundSystem() : CLogger(this), m_iNumStreams(SMITH_AUDIOSTREAMS) {}
    bool Init(int plat);
    void Shutdown();

    bool PlaySound(const std::string& name, float m_flVolume = 1.f, float m_flPan = 0.f, bool m_bLoop = false);
    bool PlayPositional(const std::string& name, const Vector2& source, float min_vol = 0.1f, float max_vol = 0.85f);

    int Loop();
private:
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
    CStreamGroup<SMITH_AUDIOSTREAMS> streams;
    
};