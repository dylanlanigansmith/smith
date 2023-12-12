#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>

#include <util/misc.hpp>
#include <data/CAnimData.hpp>




class CAnimationSystem : public CBaseInterface
{
    friend class CEditor; friend class CResourceSystem;
public:
    CAnimationSystem() : CBaseInterface("IAnimationSystem") { }
    ~CAnimationSystem() override;
    virtual void OnCreate() override;
    virtual void OnShutdown() override;
    virtual void OnLoopStart() override;
    virtual void OnLoopEnd() override;
    virtual void OnRenderStart() override{}
    virtual void OnRenderEnd() override {}
    virtual void OnEngineInitFinish() override {}

    CAnimData* GetSequence(const std::string& parent_name, const std::string& seq_name);

    void RegisterController() {} //have this system handle surface creation and allocation??

    [[nodiscard]] std::unique_ptr<CAnimSurface> GetSurface(const IVector2& size);
private:
    nlohmann::json ListToJson();
    void ListFromJson(const nlohmann::json& js);
    void LoadSequences();

private:
   std::unordered_map<std::string, CAnimData*> animation_list;

};
