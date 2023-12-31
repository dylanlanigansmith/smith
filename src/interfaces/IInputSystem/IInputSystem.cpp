#include "IInputSystem.hpp"
#include <engine/engine.hpp>
CInputSystem::~CInputSystem()
{
}

void CInputSystem::OnCreate()
{
    m_devMenuOpen = false;
    keyboardState = SDL_GetKeyboardState(&keyboardSize);
    m_flSensitivity = 5.2; // 2.24 @ 800 seems good
    m_ySensitivity = m_flSensitivity + 1.2;
    m_flMouseScale = 0.0005;
    m_flMouseAccel = 1.0;
    m_bMouseLook = true;
    m_bGrabCursor = true;
    m_bPitch = true;
    if (m_bMouseLook && m_bGrabCursor)
        SDL_SetRelativeMouseMode(SDL_TRUE);

    status("init. sens=%.3f, mouselook=%d, grabcursor=%d", m_flSensitivity, m_bMouseLook, m_bGrabCursor);
   
}

void CInputSystem::OnShutdown()
{
}

void CInputSystem::OnLoopStart()
{
    m_wasd.w = m_wasd.a = m_wasd.s = m_wasd.d = false;
    m_vecMouseMove = Vector2();
    // SDL_PumpEvents();
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
        OnMouseMotion(event);
        return;
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
    auto mode = SDL_GetRelativeMouseMode();
    if (mode == SDL_FALSE && code != SDL_SCANCODE_BACKSLASH && m_bGrabCursor)
        return false;
    if (code > keyboardSize - 1)
        return false;

    return keyboardState[code];
}

bool CInputSystem::IsMouseButtonDown(uint8_t button)
{
    auto mode = SDL_GetRelativeMouseMode();
    if (mode == SDL_FALSE)
        return false;
    uint32_t buttons;
    buttons = SDL_GetMouseState(NULL, NULL);

    switch (button)
    {
    case 1:
        return ((buttons & SDL_BUTTON_RMASK) != 0);
    case 0:
    default:
        return ((buttons & SDL_BUTTON_LMASK) != 0);
    }
}

Vector2 CInputSystem::GetLastMouseMove()
{


    static auto lastTick = IEngineTime->GetCurRenderTick();
    auto curTick =  IEngineTime->GetCurRenderTick();
    auto updates = std::max( curTick - lastTick, (looptick_t)1);
 //   updates = 1;
    Vector2 lastMove = m_vecMouseMove / (double)updates;
    m_vecMouseMove = {0.f, 0.f};
    lastTick = IEngineTime->GetCurRenderTick();
    return lastMove;
  
}

double SensitivityCurve(double delta, double sensitivity, double acceleration)
{
    double sign = (delta >= 0) ? 1.0 : -1.0;
    double magnitude = abs(delta);

    // Apply the non-linear scaling
    double adjustedMagnitude = pow(magnitude, acceleration);

    // Reapply the sign and adjust with sensitivity factor
    return sign * adjustedMagnitude * sensitivity * 10;
}

double SensitivityCurve2(double delta, double sensitivity, double acceleration, double scale)
{
     double sign = (delta >= 0) ? 1.0 : -1.0;
    // Reapply the sign and adjust with sensitivity factor
    return scale *  (sensitivity  + (abs(delta) * acceleration)) * sign;
}
double SensitivityCurve3(double delta, double sensitivity, double acceleration, double scale)
{
    
    // Reapply the sign and adjust with sensitivity factor
    return scale *  (sensitivity  * delta );
}
void CInputSystem::OnMouseMotion(SDL_Event *event)
{
    auto mode = SDL_GetRelativeMouseMode();
    if (mode == SDL_FALSE && m_bGrabCursor)
        return;

    ///  if(abs(event->motion.xrel) < 1 && abs(event->motion.yrel) < 1)
    //  return;
    // log("%f", event->motion.xrel);
    if (event->motion.xrel > 0.f || event->motion.xrel < 0.f)
    {
        // log("%f", event->motion.xrel);
    }
    m_vecMouseMove = {
        m_vecMouseMove.x +  SensitivityCurve3(event->motion.xrel, m_flSensitivity * 0.5 , m_flMouseAccel, m_flMouseScale) * -1.0,
         m_vecMouseMove.y + SensitivityCurve3(event->motion.yrel, m_ySensitivity * 0.5, m_flMouseAccel, m_flMouseScale) * -1.0};
    /*
    //accel
     m_vecMouseMove = {
        m_flMouseScale * SensitivityCurve(event->motion.xrel, m_flSensitivity, m_flMouseAccel) * -1.0,
        m_flMouseScale * SensitivityCurve(event->motion.yrel, m_flSensitivity, m_flMouseAccel)};

        //raw
        m_vecMouseMove = {
            m_vecMouseMove.x + event->motion.xrel * m_flSensitivity * m_flMouseScale * -1.0,
            m_vecMouseMove.y + event->motion.yrel * m_flSensitivity * m_flMouseScale
        };*/
    
}

void CInputSystem::OnKeyDown(SDL_Keycode code)
{
    switch (code)
    {
    case SDL_KeyCode::SDLK_ESCAPE:
        auto mode = SDL_GetRelativeMouseMode();
        SDL_SetRelativeMouseMode((mode == SDL_TRUE) ? SDL_FALSE : SDL_TRUE);
        break;
    }
}
