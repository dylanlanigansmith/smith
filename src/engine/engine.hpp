#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>
#include <logger/logger.hpp>
#include <renderer/renderer.hpp>
#include <interfaces/interfaces.hpp>
#include <enet/enet.h>
#include <audio/audio.hpp>
//#define SMITHNETWORKED

struct smith_sys_info
{
    int sys_cores;
    int render_threads_to_use;
    int sys_l1_line_size;
    int sys_ram;
    std::string plat_name;
    void find(){
        render_threads_to_use = 0;
        sys_cores = SDL_GetCPUCount();
        sys_l1_line_size = SDL_GetCPUCacheLineSize();
        sys_ram = SDL_GetSystemRAM();
        plat_name = SDL_GetPlatform();

        //1 thread for gameloop, 1 thread for audio, N threads for render (need 8 tbh, 500fps mandatory)
        if(sys_cores > 12 && SCREEN_HEIGHT > 360) render_threads_to_use = 10; 
        else if(sys_cores > 10) render_threads_to_use = 8; //likely 12 available 
        else if(sys_cores > 6) render_threads_to_use = 4;
        else if(sys_cores > 2) render_threads_to_use = 2; //all 4 cores a blazin
        else render_threads_to_use = 0;
        //this is arbitrary and likely wrong :)

    }
};

class CEngine : public CLogger
{
public:
    CEngine() : CLogger(std::string("Engine")) {shouldStopLoop = SDL_TRUE; }
    virtual ~CEngine();

    void Start(const char* title);
    int Run();
    
    auto window() const { return m_SDLWindow; }

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
    auto& GetSysInfo() const { return m_sysInfo; }
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
    smith_sys_info m_sysInfo;
};

extern CEngine* engine;