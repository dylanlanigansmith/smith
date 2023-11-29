#include "IInputSystem.hpp"

CInputSystem::~CInputSystem()
{
}

void CInputSystem::OnCreate()
{
    keyboardState = SDL_GetKeyboardState(&keyboardSize);
    m_flSensitivity = 2.0;
    m_bMouseLook = true;
    m_bPitch = false;
    if(m_bMouseLook)
        SDL_SetRelativeMouseMode(SDL_TRUE);
}

void CInputSystem::OnShutdown()
{
}

void CInputSystem::OnLoopStart()
{
    m_wasd.w = m_wasd.a = m_wasd.s = m_wasd.d = false;
    m_vecMouseMove = Vector2();
    SDL_PumpEvents();
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
        case SDL_EVENT_MOUSE_MOTION:
            OnMouseMotion(event); return;
        default:
            return;
    }
}

WASD_t CInputSystem::GetInput()
{
    m_wasd = {
        .w = IsKeyDown(SDL_SCANCODE_W),
        .a = IsKeyDown(SDL_SCANCODE_A),
        .s = IsKeyDown(SDL_SCANCODE_S),
        .d = IsKeyDown(SDL_SCANCODE_D),
    };
    return m_wasd;
}

bool CInputSystem::IsKeyDown(SDL_Scancode code)
{
    if(code > keyboardSize - 1)
        return false;

    return keyboardState[code];
}



void CInputSystem::OnMouseMotion(SDL_Event* event)
{
    auto mode = SDL_GetRelativeMouseMode();
    if(mode == SDL_FALSE) return;

    const double flScaleFactor = 0.2;
    if(abs(event->motion.xrel) < 1 && abs(event->motion.yrel) < 1)
        return;
    m_vecMouseMove = {
        event->motion.xrel * m_flSensitivity * flScaleFactor * -1.0,
        event->motion.yrel * m_flSensitivity * flScaleFactor
    };

}

void CInputSystem::OnKeyDown(SDL_Keycode code)
{
    switch (code)
    {
        case SDL_KeyCode::SDLK_ESCAPE:
            auto mode = SDL_GetRelativeMouseMode();
            SDL_SetRelativeMouseMode( (mode == SDL_TRUE) ? SDL_FALSE : SDL_TRUE) ;
            break;
        
    }
}
