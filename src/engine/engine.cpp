#include "engine.hpp"
#include <interfaces/interfaces.hpp>
#include <imgui_impl_sdl3.h>
#include <magic_enum/magic_enum.hpp>

#include <ctime>
CEngine::~CEngine()
{
}

bool CEngine::Start(const char* title)
{
    log("--smith-engine--");
    

    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        Error("SDLInit failed: %s. Off to a great start I see!", SDL_GetError()); 
        PLATFORM.Dialog().MessageBox("SDLInit failed: %s. Off to a great start I see!", SDL_GetError());
        return false;
    }   
    log("starting window { %ix%i }", SCREEN_WIDTH_FULL, SCREEN_HEIGHT_FULL);
    if(!PLATFORM.window().SetupForPlatform(PLATFORM.GetPlatType())){
         PLATFORM.Dialog().MessageBox("Window Setup Failed!");  return false;
    }
    if(!PLATFORM.window().CreateWindow(title, SCREEN_WIDTH_FULL, SCREEN_HEIGHT_FULL)){
        Error("SDL_CreateWindow failed: %s", SDL_GetError()); 
        PLATFORM.Dialog().MessageBox("SDL_CreateWindow failed: %s", SDL_GetError());
        return false; 
    }
    m_SDLWindow = PLATFORM.window().m_SDLWindow; //this is something we can have now

    if(GetSysInfo().render_threads_to_use < 8)
         warn("system core count does not meet the recommended specifications"); //need that 700fps
    
    render = new CRenderer(m_SDLWindow);
    if(!render->Create()) return false; //should do sdl quit etc even on fail
   
    if(PLATFORM.IsLinux()) //do thread priority
    {
        SDL_ThreadPriority priority = SDL_THREAD_PRIORITY_HIGH;
        if(int err = SDL_SetThreadPriority(priority); err != 0){
            Error("Failed to set thread-priority '%s'  : (%i) %s ",magic_enum::enum_name(priority).data(), err, SDL_GetError() );
        } else{
            log("set threadpriority -> %s", magic_enum::enum_name(priority).data() ); //+10-15fps boost with -Og  -> should we do this now that we are multithreaded
        }
    }
    
    //seed the bad rng 
    srand(  time(nullptr) );

    
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

    //circular dependency hell
    for(auto& element : interfaces.list())
            element.second->OnResourceLoadStart();
    for(auto& element : interfaces.list())
            element.second->OnResourceLoadEnd();
    for(auto& element : interfaces.list())
            element.second->OnEngineInitFinish();
    render->OnEngineInitFinish();

    return true;
}

int CEngine::Run()
{
    static auto IInputSystem = CreateInterface<CInputSystem>("IInputSystem");
    static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
    static auto UpdateProfiler = IEngineTime->AddProfiler("Engine::PostRenderUpdate()");
    static auto TotalRenderProfiler = IEngineTime->AddProfiler("Engine::RenderLoop()");
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
            if(event.type == SDL_EVENT_KEY_DOWN && event.key.keysym.scancode == SDL_SCANCODE_F10){
                shouldStopLoop = SDL_TRUE;
                break;
            }
            IInputSystem->OnEvent(&event);
        }
        
        for(auto& element : interfaces.list())
            element.second->OnRenderStart();
        TotalRenderProfiler->Start();
        render->Loop();
        TotalRenderProfiler->End();
        UpdateProfiler->Start();
        for(auto& element : interfaces.list())
            element.second->OnRenderEnd();
        for(auto& element : interfaces.list())
            element.second->OnLoopEnd();
        UpdateProfiler->End();
       // log("%f", 1.0 / IEngineTime->GetLastFrameTime().sec());//this is broken
    }
   
   
    m_SoundSystem.Shutdown();
     Shutdown();
    return 0;
}
int CEngine::Shutdown()
{
    interfaces.Destroy();

#ifdef SMITH_NETWORKED 
    enet_deinitialize();
#endif
    render->Shutdown();
    warn("smith-engine shutdown");
    SDL_DestroyWindow(m_SDLWindow);
    SDL_Quit();

    return 0;
}


void CEngine::InitInterfaces()
{
    //CoreEngine
    interfaces.AddInterface<CFileSystem>();
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



