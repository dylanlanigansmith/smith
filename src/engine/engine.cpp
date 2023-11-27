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
    static auto IInputSystem = CreateInterface<CInputSystem>("IInputSystem");
    static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");

    while(!shouldStopLoop)
    {
        for(auto& element : interfaces.list())
            element.second->OnLoopStart();
        


        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if(event.type == SDL_EVENT_QUIT){
                shouldStopLoop = SDL_TRUE;
                break;
            }
            //https://wiki.libsdl.org/SDL3/SDL_GetKeyboardState
            IInputSystem->OnEvent(&event);
        }
        
        for(auto& element : interfaces.list())
            element.second->OnRenderStart();
        render->Loop();
        for(auto& element : interfaces.list())
            element.second->OnRenderEnd();
        for(auto& element : interfaces.list())
            element.second->OnLoopEnd();

       // log("%f", 1.0 / IEngineTime->GetLastFrameTime().sec());
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
    interfaces.AddInterface<CInputSystem>();
    interfaces.AddInterface<CResourceSystem>();
    interfaces.AddInterface<CTextureSystem>();
    interfaces.AddInterface<CEntitySystem>();
    interfaces.AddInterface<CLevelSystem>();
}



