#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>
#include <types/Vector.hpp>
#include <data/Texture.hpp>
#include "CAnimationSequence.hpp"
#include <logger/logger.hpp>
#include <renderer/renderer.hpp>
/*
http://www.doomlegends.com/emporium/tutorials/sprite_basics.html

*/
struct anim_surface_t{
    
    SDL_Surface* m_surface;
    texture_t* m_sourceTexture;
    SDL_Rect m_curRect;
    SDL_Rect m_defaultRect;
    SDL_Color m_clrKey;
    SDL_Color m_clrKey2;
};

struct anim_params_t
{
    std::string m_szName; //16 bytes
    IVector2 m_surfaceSize;
    SDL_Color m_clrKey;
    SDL_Color m_clrKey2;
};


class CAnimController : public CLogger
{
public:
    CAnimController(const std::string& m_szTextureName, const anim_params_t& params, const CAnimSequence& defseq) 
        :  CLogger(this, params.m_szName), m_params(params)  {
        m_draw.m_defaultRect = {0,0, m_params.m_surfaceSize.w(), m_params.m_surfaceSize.h()};
        m_draw.m_clrKey = m_params.m_clrKey;
        m_draw.m_clrKey2 = m_params.m_clrKey2;
        AddSequence(defseq, true);
       
        SetupTexture(m_szTextureName); m_curUpdate = 0;
    }
    virtual ~CAnimController() {
        OnDestroy();
    }
    virtual void OnUpdate();
    virtual void DrawFrame(CRenderer* renderer, IVector2 offset, uint8_t alpha = 255);
    virtual bool AddSequence(const CAnimSequence& seq, bool is_default = false);
    virtual void PlaySequence(hSequence request_seq);

    virtual anim_surface_t* Drawable() { return &m_draw; }

    virtual bool SwitchFrames(CAnimSequence* seq);
protected:
    virtual void NextFrame();
    virtual void SetupTexture(const std::string& m_szTextureName);
    virtual void OnDestroy();

private:
   
    anim_surface_t m_draw;
    anim_params_t m_params;
    uint64_t m_curUpdate;
    uint64_t m_nextUpdate;

    hSequence m_defaultSequence;
    CAnimSequence* m_pDefaultSequence;

    hSequence m_curSequence;
    CAnimSequence* m_pCurSequence;
    std::unordered_map<hSequence, CAnimSequence> m_sequences;
};

