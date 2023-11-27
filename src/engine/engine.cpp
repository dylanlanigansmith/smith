#include "engine.hpp"

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
        render->Loop();
    }

    return Shutdown();
}
int CEngine::Shutdown()
{
    for(auto& entry : interface_list)
    {
        auto interface = entry.second;
        interface->Shutdown();
        delete interface;
    }
    delete render;
    SDL_DestroyWindow(m_SDLWindow);
    SDL_Quit();

    return 0;
}


void CEngine::InitInterfaces()
{
   
}



std::unique_ptr<CBaseInterface> CEngine::CreateInterface(const std::string &name)
{
    auto search = interface_list.find(name);
    if(search != interface_list.end())
    {
        return std::unique_ptr<CBaseInterface>(interface_list.at(name));
    }
    log("create interface not found for %s", name.c_str());
    return std::unique_ptr<CBaseInterface>(nullptr);
}