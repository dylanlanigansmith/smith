#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <types/CTime.hpp>

class CEngineTime : public CBaseInterface
{
public:
    CEngineTime() : CBaseInterface("IEngineTime") {}
    ~CEngineTime();
    virtual void OnCreate() override;
    virtual void OnShutdown() override;

private:
    uint64_t m_loopStart;
    uint64_t m_curTime;
};
