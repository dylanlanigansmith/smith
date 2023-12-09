#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <SDL3/SDL.h>
#include <types/Vector.hpp>

struct WASD_t
{
    bool w,a,s,d;
};

class CInputSystem : public CBaseInterface
{
    friend class CEditor;
public:
    CInputSystem() : CBaseInterface("IInputSystem") { }
    ~CInputSystem() override;
    virtual void OnCreate() override;
    virtual void OnShutdown() override;
    virtual void OnLoopStart() override;
    virtual void OnLoopEnd() override;
    virtual void OnRenderStart() override{}
    virtual void OnRenderEnd() override {}
    virtual void OnEvent(SDL_Event* event );


    [[nodiscard]] virtual WASD_t GetInput();
    virtual bool IsKeyDown(SDL_Scancode code);
    virtual bool IsMouseButtonDown(uint8_t button );
    [[nodiscard]] virtual bool UseMouseMovement() const { return m_bMouseLook; }
    virtual Vector2 GetLastMouseMove();
    const auto AllowPitch() { return m_bPitch; }
private:
    virtual void OnMouseMotion(SDL_Event* event);
    virtual void OnKeyDown(SDL_Keycode code);
private:
    bool m_bGrabCursor;
    Vector2 m_vecMouseMove;
    double m_flSensitivity;
    double m_flMouseScale;
    double m_flMouseAccel;
    bool m_bMouseLook;
    bool m_bPitch;
    WASD_t m_wasd;
    const uint8_t* keyboardState;
    int keyboardSize;
};
