#include "CAnimationController.hpp"
#include <engine/engine.hpp>


void CAnimController::DrawFrame(CRenderer *renderer, IVector2 offset, uint8_t alpha ) //provide a callback function for pixel manip as an arg!!!!
{
     auto animd = &(this->m_draw);
    if(offset.x != 0 || offset.y != 0 )
        this->m_vecOffset = offset;
    const int frame_width = animd->m_curRect.w; //80
    const int frame_height = animd->m_curRect.h; //80

    int start_x = SCREEN_WIDTH / 2 - frame_width / 2 + m_vecOffset.x;
    int start_y = SCREEN_HEIGHT - frame_height + m_vecOffset.y;


        auto worldPos = m_pParent->GetPosition();
       auto tile = ILevelSystem->GetTileAtFast(worldPos.x, worldPos.y);
    // greasy
    texture_t t;
    t.m_texture = animd->m_surface;

    const SDL_Color mask = animd->m_clrKey;
    const SDL_Color mask2 = animd->m_clrKey2; //blk
    for (int y = 0; y < frame_height; ++y)
        for (int x = 0; x < frame_width; ++x)
        {
            auto color = t.getColorAtPoint({x, y});
            SDL_Color clr = Render::TextureToSDLColor(color);
            if (!color)
                continue;
            if (Render::ColorEqualRGB(clr, mask))
                continue;
            if (Render::ColorEqualRGB(clr, mask2))
                continue;
             if (m_draw.m_sourceTexture->m_clrKey && Render::ColorEqualRGB(clr, Render::TextureToSDLColor(m_draw.m_sourceTexture->m_clrKey)))
                continue;
            if(clr.g > 160 && clr.r < 78 && clr.b < 78 )
                continue;
            if(alpha == 255)
                renderer->SetPixel(start_x + x, start_y + y, color);
            else{
                auto fixed  = clr; fixed.a = alpha;
                auto behind = renderer->GetPixel(start_x + x, start_y + y);
               // log("%i %i %i", behind.r, behind.g, behind.g);
                 ILightingSystem->ApplyLightForTile(tile, true, true, worldPos, start_x + x, start_y + y);
                 renderer->SetPixel(start_x + x, start_y + y, Render::MergeColorsFast(fixed, behind)); //i have to do this manually...
            }
            
        }
}

IVector2 CAnimController::ScreenToOffset(IVector2 screen)
{
     auto animd = &(this->m_draw);
    
    const int frame_width = animd->m_curRect.w; //80
    const int frame_height = animd->m_curRect.h; //80
     return {
        screen.x - SCREEN_WIDTH / 2 + frame_width / 2,
        screen.y - SCREEN_HEIGHT + frame_height
     };

   
}

void CAnimController::NextFrame()
{
    if(m_pCurSequence == nullptr){
        Error("no current sequence! %x", m_curSequence); return;
    }

    m_pCurSequence->startframe();
    //SDL_BlitSurface(m_draw.m_sourceTexture->m_texture, &(m_pCurSequence->cur()->rect), m_draw.m_surface, &m_draw.m_curRect);
    SwitchFrames(m_pCurSequence);

    if(m_pCurSequence->last()){
        //log("last");
        m_nextUpdate = 0;
        m_curSequence = -1;
        m_pCurSequence->reset();
        m_pCurSequence = nullptr; 
        return;
    }
    //dbg("#%i %s", m_pCurSequence->cur()->num, m_pCurSequence->name().c_str());
   
    m_nextUpdate = m_curUpdate + m_pCurSequence->m_frameTime;
    m_pCurSequence->endframe();
}

void CAnimController::OnUpdate()
{
    m_curUpdate++;
  //  if(m_curSequence == m_defaultSequence && m_pCurSequence == nullptr) // no ptr for when it is default
   //     return;
    if(m_nextUpdate == 0) {
        if(m_curSequence == HSEQUENCE_INVALID && m_pCurSequence == nullptr){
            m_curSequence = m_defaultSequence;
            m_pCurSequence = m_pDefaultSequence;
            SwitchFrames(m_pDefaultSequence);
        }
        return;
    }

    if(m_nextUpdate > m_curUpdate ) return;

    NextFrame();
    
}
void CAnimController::OnDestroy()
{
    SDL_DestroySurface(m_draw.m_surface);
    m_sequences.clear();
}


bool CAnimController::SwitchFrames(CAnimSequence *seq)
{
    auto* cur = seq->cur();
    // SDL_Rect result = {cur->pos.x,cur->pos.y,0,0};
   //  SDL_Rect* destRect = &result;
   // if(cur->pos.x == -1 && cur->pos.y == -1)
    //    destRect = NULL;

   // auto& sr = seq->cur()->rect;
   // dbg("src rect {%i %i %i %i}",sr.x,sr.y,sr.w,sr.h );
    int code = SDL_BlitSurfaceScaled(m_draw.m_sourceTexture->m_texture, &(cur->rect), m_draw.m_surface, NULL); //mess here is for future position moving
     // auto& dr = *result;
   //  dbg("blit rect {%i %i %i %i}",dr.x,dr.y,dr.w,dr.h );
   m_draw.m_curRect =  {0,0,m_draw.m_surface->w, m_draw.m_surface->h};
   // m_draw.m_curRect = cur->rect; // (destRect == NULL) ? SDL_Rect(0,0,m_draw.m_surface->w, m_draw.m_surface->h) :  *destRect;
    if(code  == 0)
        return true;
    auto err = SDL_GetError();
   
   
    Error("switchframe/blit failed: %s %i",err, code );
    return false;
}

void CAnimController::SetupTexture(const std::string& m_szTextureName)
{
    m_draw.m_sourceTexture = ITextureSystem->FindOrCreatetexture(m_szTextureName);
    dbg("texture found %s %i", m_szTextureName.c_str(), m_draw.m_sourceTexture->m_texture != nullptr);

    m_draw.m_surface = SDL_CreateSurface(m_params.m_surfaceSize.w(),m_params.m_surfaceSize.h(), SMITH_PIXELFMT);
    if(m_draw.m_surface == NULL){
        auto err =  SDL_GetError();
        Error(" setuptexture: %s", err);
    }
    SwitchFrames(m_pDefaultSequence);
}

bool CAnimController::AddSequence(const CAnimSequence& seq, bool is_default)
{
    const std::string seq_name = seq.name();
    auto success = m_sequences.emplace(seq.handle(), seq);
    if(is_default){
        m_defaultSequence =  seq.handle();
        m_pDefaultSequence = (success.second) ? &(success.first->second) : nullptr;
        m_draw.m_defaultRect = m_pDefaultSequence->at(0)->rect;
        auto& dr = m_draw.m_defaultRect;
        dbg("setting default seq to %s {%i %i %i %i}", seq_name.c_str(),dr.x,dr.y,dr.w,dr.h );
    }
    
    if(!success.second)
        Error("failed to add sequence %s %i", seq_name.c_str(), (int)is_default);
    else
        log("added sequence %s,  frames {%li} start{%i}", seq_name.c_str(), seq.m_frames.size(), seq.m_curFrame);
    return success.second;
}

void CAnimController::PlaySequence(hSequence request_seq, IVector2 offset)
{
    m_curSequence = request_seq;
    m_pCurSequence = &(m_sequences.at(request_seq));

    if(m_pCurSequence == nullptr || (uintptr_t)m_pCurSequence < 0xFF){
        Error("PlaySequence: could not find or set newsequence %x", m_curSequence); //return;
    }
    m_nextUpdate = m_curUpdate; // + m_pCurSequence->m_frameTime;
    if(offset.x != 0 || offset.y != 0 )
        m_vecOffset = offset;

}