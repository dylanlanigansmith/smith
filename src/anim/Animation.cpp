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



CAnimData* CAnimation::AddDefaultSequenceByName(const std::string &seq_name, const IVector2& size_override)
{
    if(m_defaultSequence != nullptr){
        Error("overwriting default sequence with %s", seq_name.c_str());
    }
   m_defaultSequence = IAnimationSystem->GetSequence(m_szName, seq_name);
   if(m_defaultSequence == nullptr){
        Error("no def seq found for %s", seq_name.c_str()); return nullptr;
   }
    m_surface = (size_override.x == -1 && size_override.y == -1) ? IAnimationSystem->GetSurface(m_defaultSequence->GetSize()) : IAnimationSystem->GetSurface(size_override) ;
    AddKnownSequence(m_defaultSequence);
    dbg("acquired surface %s [%d %d] and switched to default sequence [0]",(size_override.x == -1) ? "size" : "override", m_surface->w(), m_surface->h()); 
    m_curSequence = m_defaultSequence;
    SwitchFrames(m_defaultSequence, 0); m_curFrame = 0;

    return m_defaultSequence;
}

CAnimData* CAnimation::AddSequenceByName(const std::string &seq_name)
{
   auto to_add = IAnimationSystem->GetSequence(m_szName, seq_name);
   if(m_defaultSequence == nullptr){
        Error("no seq found for %s", seq_name.c_str()); return nullptr;
   }
    AddKnownSequence(to_add);
    return to_add;
}

void CAnimation::PlaySequenceByName(const std::string &seq_name, bool no_interupt )
{
    if(m_curSequence != m_defaultSequence  && no_interupt) return; //HACKY 
    m_curSequence = GetSequenceByLocalName(seq_name);

    SwitchFrames(m_curSequence, 0);
   
    OnSequenceStart(seq_name);
    if(!m_curSequence->IsAnimNoAutoplay())
        m_nextUpdate = m_curUpdate + m_curSequence->GetRate();
    else return;

   // log(seq_name);
}


void CAnimation::SwitchFrames(CAnimData *seq, int idx)
{
    //ahahaha remember when we used to check for errors here 
    auto surf = seq->GetSurface();
    if(m_overrideTexture != nullptr)
        surf = m_overrideTexture->m_texture;

    SDL_FillSurfaceRect(*m_surface, NULL, 0);
    if(seq->IsAnimNoScale())
    {
        auto& fr = seq->GetFrames()[idx].m_rect;
        SDL_Rect no_scale = {0,0,fr.w, fr.h };
        if(seq->IsAnimCentered()){
            no_scale.x = m_surface->w() - (fr.w / 2);
            no_scale.y = m_surface->h() - (fr.h / 2); //do we really wanna do this?
        }
        SDL_BlitSurface(surf, &(seq->GetFrames()[idx].m_rect), *m_surface, &(no_scale));
    }
    else
        SDL_BlitSurfaceScaled(surf, &(seq->GetFrames()[idx].m_rect), *m_surface, NULL);
    
}


