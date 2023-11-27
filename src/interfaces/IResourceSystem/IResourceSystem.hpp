#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>


class CResourceSystem : public CBaseInterface
{
public:
    CResourceSystem() : CBaseInterface("IResourceSystem") { }
    ~CResourceSystem() override;
    virtual void OnCreate() override;
    virtual void OnShutdown() override;
    virtual void OnLoopStart() override {}
    virtual void OnLoopEnd() override {}
    virtual void OnRenderStart() override {}
    virtual void OnRenderEnd() override {}
    virtual std::string GetResourcePath();
    virtual std::string GetResourceSubDir(const std::string& folder);
private:
    std::string m_szResourcePath;
    std::string m_szHomeDir;
};
