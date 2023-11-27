#include "IInputSystem.hpp"

CInputSystem::~CInputSystem()
{
}

void CInputSystem::OnCreate()
{
}

void CInputSystem::OnShutdown()
{
}

void CInputSystem::OnLoopStart()
{
    m_wasd.w = m_wasd.a = m_wasd.s = m_wasd.d = false;
}

void CInputSystem::OnLoopEnd()
{
}

void CInputSystem::OnEvent(SDL_Event *event)
{
    switch (event->type)
    {
        case SDL_EVENT_KEY_DOWN:
            OnKeyDown(event->key.keysym.sym);
            return;
        default:
            return;
    }
}

WASD_t CInputSystem::GetInput()
{
    return m_wasd;
}

void CInputSystem::OnKeyDown(SDL_Keycode code)
{
    switch (code)
    {
        case SDL_KeyCode::SDLK_w:
            m_wasd.w = true; return;
        case SDL_KeyCode::SDLK_a:
            m_wasd.a = true; return;
        case SDL_KeyCode::SDLK_s:
            m_wasd.s = true; return;
        case SDL_KeyCode::SDLK_d:
            m_wasd.d = true; return;
    }
}
