#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <types/CTime.hpp>
#include <util/misc.hpp>


#include "CProfiler.hpp"




class CEngineTime : public CBaseInterface
{
public:
    friend class CEntitySystem; //this needs work
     friend class CEditor;
    CEngineTime() : CBaseInterface("IEngineTime") { }
    ~CEngineTime() override;
    virtual void OnCreate() override;
    virtual void OnShutdown() override;
    virtual void OnLoopStart() override;
    virtual void OnLoopEnd() override;
    virtual void OnRenderStart() override{}
    virtual void OnRenderEnd() override {}
    virtual float GetFPS();
    virtual float GetFPSAvg();
    virtual Time_t GetCurTime();
    virtual Time_t GetLastFrameTime();
    virtual looptick_t GetCurLoopTick();
    virtual looptick_t GetCurRenderTick() const { return m_renderticks; }
    virtual Timer_t& GetUpdateTimer() { return m_updateTimer; }

    CProfiler* AddProfiler(const std::string& m_szProfilerName){
        auto prof = new CProfiler(this, m_szProfilerName);
        auto result = profilers.emplace(m_szProfilerName, prof);
        if(!result.second)
            Error("failed to add profiler %s", m_szProfilerName.c_str());
        return prof;
    }
    CProfiler* GetProfiler(const std::string& m_szProfilerName){
        return profilers.at(m_szProfilerName);
    }

    
private:
    looptick_t m_loopticks;
    looptick_t m_renderticks;
    Timer_t m_updateTimer;
    Timer_t m_loopTimer;
    Time_t m_lastFrameTime;

    Time_t m_loopStart;
    Time_t m_curTime;

    std::unordered_map<std::string, CProfiler*> profilers;
};

