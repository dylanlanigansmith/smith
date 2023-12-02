#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <types/CTime.hpp>

#define TICKS_PER_S 32

class CEngineTime : public CBaseInterface
{
public:
    friend class CEntitySystem;
    CEngineTime() : CBaseInterface("IEngineTime") { }
    ~CEngineTime() override;
    virtual void OnCreate() override;
    virtual void OnShutdown() override;
    virtual void OnLoopStart() override;
    virtual void OnLoopEnd() override;
    virtual void OnRenderStart() override{}
    virtual void OnRenderEnd() override {}
    virtual Time_t GetCurTime();
    virtual Time_t GetLastFrameTime();
    virtual looptick_t GetCurLoopTick();
    virtual Timer_t& GetUpdateTimer() { return m_updateTimer; }
private:
    looptick_t m_loopticks;
    Timer_t m_updateTimer;
    Timer_t m_loopTimer;
    Time_t m_lastFrameTime;

    Time_t m_loopStart;
    Time_t m_curTime;
};
