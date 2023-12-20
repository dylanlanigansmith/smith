#include "engine.hpp"
#include <interfaces/interfaces.hpp>
#include <imgui_impl_sdl3.h>
#include <magic_enum/magic_enum.hpp>

#include <ctime>

CTextureSystem* ITextureSystem = nullptr;
CEngineTime* IEngineTime = nullptr;
CInputSystem* IInputSystem = nullptr;
CResourceSystem* IResourceSystem = nullptr;
CFileSystem* IFileSystem = nullptr;
CLevelSystem* ILevelSystem = nullptr;
CEntitySystem* IEntitySystem = nullptr;
CLightingSystem* ILightingSystem = nullptr;
CAnimationSystem* IAnimationSystem = nullptr;


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
    int width = SCREEN_WIDTH_FULL, height = SCREEN_HEIGHT_FULL;
    if(PLATFORM.LaunchOptions().HasArg("width") && PLATFORM.LaunchOptions().HasArg("height")){
        width = PLATFORM.LaunchOptions().GetValue<int>("width");
        height = PLATFORM.LaunchOptions().GetValue<int>("height");

    }
    if(PLATFORM.LaunchOptions().HasArg("full"))
        PLATFORM.window().MakeFullScreen();
    
    log("starting window { %ix%i }", width, height );
    if(!PLATFORM.window().SetupForPlatform(PLATFORM.GetPlatType())){
         PLATFORM.Dialog().MessageBox("Window Setup Failed!");  return false;
    }
    PLATFORM.window().SetWindowSize( width, height );
    if(!PLATFORM.window().CreateWindow(title)){
        Error("SDL_CreateWindow failed: %s", SDL_GetError()); 
        PLATFORM.Dialog().MessageBox("SDL_CreateWindow failed: %s", SDL_GetError());
        return false; 
    }
    m_SDLWindow = PLATFORM.window().m_SDLWindow; //this is something we can have now


     if(PLATFORM.LaunchOptions().HasArg("threads") ){
       int threads_arg = PLATFORM.LaunchOptions().GetValue<int>("threads");     
       if(threads_arg > 0 && threads_arg < GetSysInfo().sys_cores - 2){
             PLATFORM.m_sysInfo.render_threads_to_use = threads_arg;
             info("using overridden thread count %d", threads_arg);
       }
       else{
        warn("launch option -threads %d ignored. invalid value", threads_arg);
       }
    }


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
    
    static auto UpdateProfiler = IEngineTime->AddProfiler("Engine::PostRenderUpdate()");
    static auto TotalRenderProfiler = IEngineTime->AddProfiler("Engine::RenderLoop()");
    m_SoundSystem.Init(0);
    shouldStopLoop = SDL_FALSE;
    auto display_mode = &PLATFORM.window().m_displayMode;
    
   
    static bool fullscreen = PLATFORM.window().GfxParams().fullscreen;
    while(!shouldStopLoop)
    {
        for(auto& element : interfaces.list())
            element.second->OnLoopStart();
        
        static auto nextFullScreenChange = IEngineTime->GetCurLoopTick(); //fullscreen code
        if(IInputSystem->IsKeyDown(SDL_SCANCODE_F11) && nextFullScreenChange < IEngineTime->GetCurLoopTick()){
            nextFullScreenChange = IEngineTime->GetCurLoopTick() + TICKS_PER_S;
            fullscreen = !fullscreen;
            if(fullscreen){
                render->SetNewFullsize({display_mode->w, display_mode->h});
                if( SDL_SetWindowFullscreen(m_SDLWindow, SDL_TRUE)  != 0){
                    Error("failed to set window fullscreen to %d %d .. %s",display_mode->w, display_mode->h, SDL_GetError() );
                } else{
                    log("set fullscreen to %d %d ",display_mode->w, display_mode->h );
                }       
            }
            else{
                render->SetNewFullsize({PLATFORM.window().width, PLATFORM.window().height});
                SDL_SetWindowFullscreen(m_SDLWindow, SDL_FALSE);
                 SDL_SetWindowSize(m_SDLWindow, PLATFORM.window().width, PLATFORM.window().height);
                if( SDL_SetWindowFullscreen(m_SDLWindow, SDL_FALSE)  != 0){
                    Error("failed to set window to windowed @ %d %d .. %s",PLATFORM.window().width, PLATFORM.window().height, SDL_GetError() );
                } else{
                   log("set windowed %d %d ",PLATFORM.window().width, PLATFORM.window().height);
                }  
            }
        }

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


    render->Shutdown();
    warn("smith-engine shutdown");
    SDL_DestroyWindow(m_SDLWindow);
    SDL_Quit();

    return 0;
}
//CreateInterface<

void CEngine::InitInterfaces()
{
    //CoreEngine
    IFileSystem = interfaces.AddInterface<CFileSystem>(IFileSystem);
    IEngineTime = interfaces.AddInterface<CEngineTime>(IEngineTime);
    IInputSystem = interfaces.AddInterface<CInputSystem>(IInputSystem);

    
    //Resource 
    IResourceSystem = interfaces.AddInterface<CResourceSystem>(IResourceSystem);
    ITextureSystem = interfaces.AddInterface<CTextureSystem>(ITextureSystem);

    //GameSystems
    IAnimationSystem = interfaces.AddInterface<CAnimationSystem>(IAnimationSystem);
    IEntitySystem = interfaces.AddInterface<CEntitySystem>(IEntitySystem);
     
    //Users of GameSystems
    ILevelSystem = interfaces.AddInterface<CLevelSystem>(ILevelSystem);

    
    ILightingSystem = interfaces.AddInterface<CLightingSystem>(ILightingSystem);

   
}



