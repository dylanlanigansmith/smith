#include "engine.hpp"
#include <interfaces/interfaces.hpp>


CEngine::~CEngine()
{
}

void CEngine::Start(const char* title)
{
    SDL_Init(SDL_INIT_VIDEO);
    const int width = 1024, height = 720;
    log("starting window");
    m_SDLWindow = SDL_CreateWindow(title, width, height, SDL_WINDOW_OPENGL);

    render = new CRenderer(m_SDLWindow);
    if(!render->Create()) return;
    shouldStopLoop = SDL_FALSE;

    InitInterfaces();
}

int CEngine::Run()
{
    while(!shouldStopLoop)
    {
        for(auto& element : interfaces.list())
            element.second->OnLoopStart();
        


        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    shouldStopLoop = SDL_TRUE;
                    break;
            }
        }
        
        for(auto& element : interfaces.list())
            element.second->OnRenderStart();
        render->Loop();
        for(auto& element : interfaces.list())
            element.second->OnRenderEnd();
        for(auto& element : interfaces.list())
            element.second->OnLoopEnd();
    }

    return Shutdown();
}
int CEngine::Shutdown()
{
    
    delete render;
    SDL_DestroyWindow(m_SDLWindow);
    SDL_Quit();

    return 0;
}


void CEngine::InitInterfaces()
{
    interfaces.AddInterface<CEngineTime>();

}
template <typename T> std::unique_ptr<T> CEngine::CreateInterface(const std::string& name)
{
    if(interfaces.InterfaceExists(name))
    {
        return interfaces.CreateInterface<T>(name);
    }
    log("create interface not found for %s", name.c_str());
    return std::unique_ptr<T>(nullptr);
}

