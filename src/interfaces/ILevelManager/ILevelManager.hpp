#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>


class CLevelSystem : public CBaseInterface
{
public:
    CLevelSystem() : CBaseInterface("IEngineTime") { }
    ~CLevelSystem() override;
    virtual void OnCreate() override;
    virtual void OnShutdown() override;
    virtual void OnLoopStart() override;
    virtual void OnLoopEnd() override;
    virtual void OnRenderStart() override;
    virtual void OnRenderEnd() override;
 
private:

};
