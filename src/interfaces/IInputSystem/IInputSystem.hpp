#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <SDL3/SDL.h>

struct WASD_t
{
    bool w,a,s,d;
};

class CInputSystem : public CBaseInterface
{
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
    virtual WASD_t GetInput();
private:
    virtual void OnKeyDown(SDL_Keycode code);
private:
    WASD_t m_wasd;
};
