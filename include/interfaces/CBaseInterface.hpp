#pragma once
#include <common.hpp>
#include <logger/logger.hpp>

class CBaseInterface : public CLogger
{
public:
    CBaseInterface(std::string m_szName) : CLogger(m_szName) {}
    virtual ~CBaseInterface() {};
    const auto& name() { return m_szName; }
    virtual void OnCreate() = 0;
    virtual void OnShutdown() = 0;
    virtual void OnLoopStart() = 0;
    virtual void OnLoopEnd() = 0;
    virtual void OnRenderStart() = 0;
    virtual void OnRenderEnd() = 0;
private:
    std::string m_szName;

};