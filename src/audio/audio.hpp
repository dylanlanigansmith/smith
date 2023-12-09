#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>
#include <logger/logger.hpp>
#include <queue>
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

enum SoundCommand_Types : uint8_t
{
    SoundCmd_Invalid = 0xff,
    SoundCmd_Default = 0,
    SoundCmd_Music = 1,
    SoundCmd_Effect,
    SoundCommand_SIZE
};

struct SoundCommand{
    std::string m_szName;
    float m_flVolume; //0.0.-1.0
    uint8_t m_nType; 
    bool m_bLoop;
    SoundCommand() : m_nType(SoundCmd_Invalid) {}
    SoundCommand(const std::string& m_szName, float m_flVolume = 1.f, bool m_bLoop = false) : m_szName(m_szName), m_flVolume(m_flVolume), m_nType(SoundCmd_Default), m_bLoop(m_bLoop) {}
};
class SoundQueue {
public:
    void pushCommand(const SoundCommand& command) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(command);
    }

    bool popCommand(SoundCommand& command) {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue.empty()) {
            return false;
        }
        command = queue.front();
        queue.pop();
        return true;
    }

private:
    std::mutex mtx;
    std::queue<SoundCommand> queue;
};


class  /*LCD*/CSoundSystem : public CLogger
{
public:
    CSoundSystem() : CLogger(this) {}
    bool Init(int plat);
    void Shutdown();

    bool PlaySound(const std::string& name);

private:
    int Loop(void* data);
    int GetAudioDriver(bool list = false);
    void SetupAudioDevice();
   
    void LoadAudioFile(const std::string& name, uint8_t format = 0);
    audiodata_t* GetAudioByName(const std::string& name);

    void LogAudioData(const std::string& name);
    void LogAudioDevice();
private:
    SoundQueue m_cmdQueue;
    float m_flVolumeMaster;

    std::string m_szAudioResourcePath;
    SDL_AudioStream* m_streams[3];
    std::unordered_map<std::string, audiodata_t*> soundboard;
    audiodevice_t m_device;
    SDL_Thread* m_mainThread;
    bool m_bVerbose;
    bool m_bShouldQuit;
};