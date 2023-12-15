#pragma once
#include <entity/components/CBaseEntityComponent.hpp>
#include <data/CAnimData.hpp>
#include <entity/CBaseRenderable.hpp>

class CAnimation : public CBaseComponent
{
public:
  //  CAnimation( const std::string& m_szName) : CBaseComponent( this, m_szName), m_defaultSequence(nullptr) {}
    CAnimation(CBaseRenderable* m_parent, const std::string& m_szName) : CBaseComponent(m_parent, this, m_szName), m_nextUpdate(0), m_curUpdate(0), m_defaultSequence(nullptr), m_overrideTexture(nullptr) {}
    virtual ~CAnimation() {}
    virtual void OnCreate();
    virtual void OnUpdate();

    CAnimData* AddDefaultSequenceByName(const std::string& seq_name, const IVector2& size_override = {-1,-1});
    CAnimData* AddSequenceByName(const std::string& seq_name);
    void PlaySequenceByName(const std::string& seq_name, bool no_interupt = false);
    inline auto& Drawable() { return *m_surface; }
protected:
    virtual void OnSequenceStart(const std::string& seq_name);
    virtual void OnSequenceEnd(const std::string& seq_name);
    void BackToDefault();
    void SwitchFrames(CAnimData* seq, int idx);
    void AddKnownSequence(CAnimData* seq);
    std::string GetSequenceLocalName(const std::string& fullname);

    inline CAnimData* GetSequenceByLocalName(const std::string& name) { return m_seq.at(name); } //(to the tune of let it snow) ** let it throw let it throw let it throwwwww**
protected:
    std::unordered_map<std::string, CAnimData*> m_seq;
   
    uint64_t m_nextUpdate;
     uint64_t m_curUpdate;
    int m_curFrame;
    CAnimData* m_curSequence;
    CAnimData* m_defaultSequence;
    std::unique_ptr<CAnimSurface> m_surface;
    texture_t * m_overrideTexture;
    static auto& IAnimationSystem();
};