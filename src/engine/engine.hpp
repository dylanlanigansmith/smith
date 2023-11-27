#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>
#include <logger/logger.hpp>
#include <renderer/renderer.hpp>
#include <interfaces/interfaces.hpp>

class CEngine : public CLogger
{
public:
    CEngine() : CLogger("Engine") {shouldStopLoop = SDL_TRUE; }
    virtual ~CEngine();

    void Start(const char* title);
    int Run();
    
    auto window() const { return m_SDLWindow; }

    template <typename T> std::unique_ptr<T> 
    CreateInterface(const std::string& name)
    {
        if(interfaces.InterfaceExists(name))
        {
            return interfaces.CreateInterface<T>(name);
        }
        log("create interface not found for %s", name.c_str());
        return std::unique_ptr<T>(nullptr);
    }
protected:
    void InitInterfaces();
    int Shutdown();


private:
    SDL_Window* m_SDLWindow;
    CRenderer* render;
    CInterfaceList interfaces;
    SDL_bool shouldStopLoop;
};

extern CEngine* engine;