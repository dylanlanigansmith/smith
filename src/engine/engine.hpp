#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>
#include <logger/logger.hpp>
#include <renderer/renderer.hpp>
#include <interfaces/interfaces.hpp>

#include <audio/audio.hpp>
#include <plat/platform.hpp>

//these used to all be nicely encapsualted
//and one would have to call engine->createinterface("xxxSystem") to get a ptr to them
//but it feels silly at this point
// i dont love globals but
// sometimes it aint worth overthinking
extern CTextureSystem* ITextureSystem;
extern CEngineTime* IEngineTime;
extern CInputSystem* IInputSystem;
extern CResourceSystem* IResourceSystem;
extern CFileSystem* IFileSystem;
extern CLevelSystem* ILevelSystem;
extern CEntitySystem* IEntitySystem;
extern CLightingSystem* ILightingSystem;
extern CAnimationSystem* IAnimationSystem;


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

   
    auto SoundSystem() { return &m_SoundSystem; }
    auto& GetSysInfo() const { return PLATFORM.SysInfo(); }
protected:
    void InitInterfaces();
    int Shutdown();
   

private:
    CSoundSystem m_SoundSystem;
    SDL_Window* m_SDLWindow;
    CRenderer* render;
    CInterfaceList interfaces;
    SDL_bool shouldStopLoop;
    
};

extern CEngine* engine;


