#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>
#include <logger/logger.hpp>
#include <renderer/renderer.hpp>
#include <interfaces/CBaseInterface.hpp>

class CEngine : public CLogger
{
public:
    CEngine() : CLogger("Engine") {shouldStopLoop = SDL_TRUE; }
    virtual ~CEngine();

    void Start(const char* title);
    int Run();
    
    auto window() const { return m_SDLWindow; }
    std::unique_ptr<CBaseInterface> CreateInterface(const std::string& name);
protected:
    void InitInterfaces();
    int Shutdown();


private:
    SDL_Window* m_SDLWindow;
    CRenderer* render;
    std::unordered_map<std::string, CBaseInterface*> interface_list;
    SDL_bool shouldStopLoop;
};

extern CEngine* engine;