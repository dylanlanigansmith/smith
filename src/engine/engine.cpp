#include "engine.hpp"
#include <interfaces/interfaces.hpp>
#include <imgui_impl_sdl3.h>
#include <magic_enum/magic_enum.hpp>


CEngine::~CEngine()
{
}

void CEngine::Start(const char* title)
{
    log("--smith-engine--");
    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        Error("SDLInit failed: %s. Off to a great start I see!", SDL_GetError()); return;
    }   
    m_sysInfo.find();
    log("smith init for %s , w/ cpu_cores: %i memory: %d MiB}", m_sysInfo.plat_name.c_str(), m_sysInfo.sys_cores, m_sysInfo.sys_ram);
   
    log("starting window { %ix%i }", SCREEN_WIDTH_FULL, SCREEN_HEIGHT_FULL);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, MSAA_BUFFER);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, MSAA_SAMPLES);
    SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, TEXTURE_SCALE_QUALITY, SDL_HINT_OVERRIDE);
    m_SDLWindow = SDL_CreateWindow(title, SCREEN_WIDTH_FULL, SCREEN_HEIGHT_FULL, SDL_WINDOW_OPENGL);

     note("init renderer with %d threads", m_sysInfo.render_threads_to_use);
     if(m_sysInfo.render_threads_to_use < 8) warn("system core count does not meet the recommended specifications");
     
    render = new CRenderer(m_SDLWindow);
    if(!render->Create()) return;
    

    SDL_ThreadPriority priority = SDL_THREAD_PRIORITY_HIGH;
    if(int err = SDL_SetThreadPriority(priority); err != 0){
        Error("Failed to set thread-priority '%s'  : (%i) %s ",magic_enum::enum_name(priority).data(), err, SDL_GetError() );
    } else{
        log("set threadpriority -> %s", magic_enum::enum_name(priority).data() ); //+10-15fps boost with -Og  -> should we do this now that we are multithreaded
    }

#ifdef SMITHNETWORKED 
    if(enet_initialize() != 0){
        Error("failed to init network client %s", "stopping"); Shutdown();
    }

    const int maxConnections= 8, numChannels = 2, limit_in = 0, limit_out = 0;
    client = enet_host_create(NULL,maxConnections, numChannels, limit_in, limit_out);
    if(client == nullptr){
        Error("failed to create network client %s", "stopping"); Shutdown();
    }
    status("created network client");
#endif
    InitInterfaces();
    for(auto& element : interfaces.list())
            element.second->OnResourceLoadStart();
    for(auto& element : interfaces.list())
            element.second->OnResourceLoadEnd();
    for(auto& element : interfaces.list())
            element.second->OnEngineInitFinish();
    render->OnEngineInitFinish();
}

int CEngine::Run()
{
    static auto IInputSystem = CreateInterface<CInputSystem>("IInputSystem");
    static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");

    m_SoundSystem.Init(0);
    shouldStopLoop = SDL_FALSE;
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
   
   
    m_SoundSystem.Shutdown();
     Shutdown();
    return 0;
}
int CEngine::Shutdown()
{
    enet_deinitialize();
    render->Shutdown();
    warn("smith-engine shutdown");
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
    interfaces.AddInterface<CAnimationSystem>();
    interfaces.AddInterface<CEntitySystem>();
     
    //Users of GameSystems
    interfaces.AddInterface<CLevelSystem>();

    
    interfaces.AddInterface<CLightingSystem>();

   
}



