#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>
#include <logger/logger.hpp>
#include <renderer/renderer.hpp>
#include <interfaces/interfaces.hpp>
#include <enet/enet.h>
#include <audio/audio.hpp>
#include <plat/platform.hpp>

//#define SMITHNETWORKED


class CEngine : public CLogger
{
public:
    CEngine() : CLogger(std::string("Engine")) {shouldStopLoop = SDL_TRUE; }
    virtual ~CEngine();

    bool Start(const char* title);
    int Run();
    
    auto& window()  { return m_SDLWindow; }

    template <typename T> T*
    CreateInterface(const std::string& name)
    {
        if(interfaces.InterfaceExists(name))
        {
            return interfaces.CreateInterface<T>(name);
        }
        log("create interface not found for %s", name.c_str());
        return (nullptr);
    }

    auto TextureSystem() { return ITextureSystem; }
    auto SoundSystem() { return &m_SoundSystem; }
    auto& GetSysInfo() const { return PLATFORM.SysInfo(); }
protected:
    void InitInterfaces();
    int Shutdown();
    ENetHost* client;

private:
    CSoundSystem m_SoundSystem;
    CTextureSystem* ITextureSystem;
    SDL_Window* m_SDLWindow;
    CRenderer* render;
    CInterfaceList interfaces;
    SDL_bool shouldStopLoop;
    
};

extern CEngine* engine;