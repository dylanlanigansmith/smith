#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <types/CTime.hpp>

class CEngineTime : public CBaseInterface
{
public:
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
private:
    looptick_t m_loopticks;
    Timer_t m_loopTimer;
    Time_t m_lastFrameTime;

    Time_t m_loopStart;
    Time_t m_curTime;
};
