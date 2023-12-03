#include "engine.hpp"
#include <interfaces/interfaces.hpp>
#include <imgui_impl_sdl3.h>

CEngine::~CEngine()
{
}

void CEngine::Start(const char* title)
{
    SDL_Init(SDL_INIT_VIDEO);
    log("starting window { %ix%i }", SCREEN_WIDTH_FULL, SCREEN_HEIGHT_FULL);
    m_SDLWindow = SDL_CreateWindow(title, SCREEN_WIDTH_FULL, SCREEN_HEIGHT_FULL, SDL_WINDOW_OPENGL);

    render = new CRenderer(m_SDLWindow);
    if(!render->Create()) return;
    shouldStopLoop = SDL_FALSE;

    InitInterfaces();
    for(auto& element : interfaces.list())
            element.second->OnResourceLoadStart();
    for(auto& element : interfaces.list())
            element.second->OnResourceLoadEnd();
    for(auto& element : interfaces.list())
            element.second->OnEngineInitFinish();
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
            ImGui_ImplSDL3_ProcessEvent(&event);
            if(event.type == SDL_EVENT_QUIT){
                shouldStopLoop = SDL_TRUE;
                break;
            }
            IInputSystem->OnEvent(&event);
        }
        
        for(auto& element : interfaces.list())
            element.second->OnRenderStart();
        render->Loop();
        for(auto& element : interfaces.list())
            element.second->OnRenderEnd();
        for(auto& element : interfaces.list())
            element.second->OnLoopEnd();

       // log("%f", 1.0 / IEngineTime->GetLastFrameTime().sec());//this is broken
    }

    return Shutdown();
}
int CEngine::Shutdown()
{
    render->Shutdown();

    SDL_DestroyWindow(m_SDLWindow);
    SDL_Quit();

    return 0;
}


void CEngine::InitInterfaces()
{
    //CoreEngine
    interfaces.AddInterface<CEngineTime>();
    interfaces.AddInterface<CInputSystem>();

    //Resource
    interfaces.AddInterface<CResourceSystem>();
    ITextureSystem = interfaces.AddInterface<CTextureSystem>();

    //GameSystems
    interfaces.AddInterface<CEntitySystem>();

    //Users of GameSystems
    interfaces.AddInterface<CLevelSystem>();

    
    interfaces.AddInterface<CLightingSystem>();
}



