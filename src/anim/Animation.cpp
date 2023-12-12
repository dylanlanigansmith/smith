#include "Animation.hpp"
#include <engine/engine.hpp>


void CAnimation::OnCreate()
{

}
void CAnimation::OnSequenceStart(const std::string &seq_name)
{

}
void CAnimation::OnSequenceEnd(const std::string &seq_name)
{

}
void CAnimation::BackToDefault()
{
     m_nextUpdate = 0;
    m_curSequence = m_defaultSequence;
    SwitchFrames(m_curSequence, 0);
    m_curFrame = 0;
}

void CAnimation::OnUpdate()
{
    m_curUpdate++;
    if(m_curSequence == nullptr){
        assert(m_defaultSequence != nullptr); //bc i will forget to add one
        BackToDefault();
        return;
    }

    if(m_nextUpdate > m_curUpdate) return;

    m_curFrame++;
    if(m_curFrame > m_curSequence->GetLastIndex()){
        OnSequenceEnd(m_curSequence->GetName());
        BackToDefault();
        
        return;
    }
    
    SwitchFrames(m_curSequence, m_curFrame);
    m_nextUpdate = m_curUpdate + m_curSequence->GetRate();
}
void CAnimation::AddKnownSequence(CAnimData *seq)
{
    m_seq.emplace(GetSequenceLocalName(seq->GetName()), seq);
    dbg("added sequence %s", GetSequenceLocalName(seq->GetName()).c_str());
}
std::string CAnimation::GetSequenceLocalName(const std::string &fullname)
{
    return fullname.substr(fullname.find_first_of("_") + 1); //surely this wont be an issue
}
auto &CAnimation::IAnimationSystem()
{  static auto animsys = engine->CreateInterface<CAnimationSystem>("IAnimationSystem"); return animsys; }


void CAnimation::AddDefaultSequenceByName(const std::string &seq_name)
{
    if(m_defaultSequence != nullptr){
        Error("overwriting default sequence with %s", seq_name.c_str());
    }
   m_defaultSequence = IAnimationSystem()->GetSequence(m_szName, seq_name);
   if(m_defaultSequence == nullptr){
        Error("no def seq found for %s", seq_name.c_str()); return;
   }
    m_surface =  IAnimationSystem()->GetSurface(m_defaultSequence->GetSize());
    AddKnownSequence(m_defaultSequence);
    dbg("acquired surface [%d %d] and switched to default sequence [0]", m_surface->w(), m_surface->h()); 
    m_curSequence = m_defaultSequence;
    SwitchFrames(m_defaultSequence, 0); m_curFrame = 0;
}

void CAnimation::AddSequenceByName(const std::string &seq_name)
{
   auto to_add = IAnimationSystem()->GetSequence(m_szName, seq_name);
   if(m_defaultSequence == nullptr){
        Error("no seq found for %s", seq_name.c_str()); return;
   }
    AddKnownSequence(to_add);
}

void CAnimation::PlaySequenceByName(const std::string &seq_name)
{
    if(m_curSequence != m_defaultSequence ) return; //HACKY 
    m_curSequence = GetSequenceByLocalName(seq_name);

    SwitchFrames(m_curSequence, 0);
    m_nextUpdate = m_curUpdate + m_curSequence->GetRate();
    OnSequenceStart(seq_name);
}


void CAnimation::SwitchFrames(CAnimData *seq, int idx)
{
    //ahahaha remember when we used to check for errors here 
   
    SDL_FillSurfaceRect(*m_surface, NULL, 0);
     
    SDL_BlitSurfaceScaled(seq->GetSurface(), &(seq->GetFrames()[idx].m_rect), *m_surface, NULL);
    
}


