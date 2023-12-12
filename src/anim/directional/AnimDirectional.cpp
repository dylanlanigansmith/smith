#include "AnimDirectional.hpp"
#include <engine/engine.hpp>
#include <entity/player/CPlayer.hpp>

void CAnimDirectional::Draw(CRenderer *renderer, const sprite_draw_data &data)
{
    static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");

    IVector2 offset = {0, 0};

    if (m_curSequence == nullptr)
    {
        Error("trying to draw with curSeq null %li", m_curUpdate);
        return;
    }
    auto &rect = m_curSequence->GetFrames()[m_curFrame].m_rect;
    if ( rect.w == 0 && rect.h == 0)
        return;

    auto worldPos = m_parent->GetPosition();
    auto tile = ILevelSystem->GetTileAtFast(worldPos.x, worldPos.y);
    for (int stripe = data.drawStart.x; stripe < data.drawEnd.x; stripe++)
    {
        IVector2 tex;
        tex.x = int(256 * (stripe - ((-data.renderSize.x / 2) + data.screen.x)) * m_surface->w() / data.renderSize.x) / 256;
        if(m_animflip == AnimDir_FlipH){
                    tex.x = m_surface->w() - tex.x; //tex.y = m_surface->h() - tex.y; 
        }
        // the conditions in the if are:
        // 1) it's in front of camera plane so you don't see things behind you
        // 2) it's on the screen (left)
        // 3) it's on the screen (right)
        // 4) ZBuffer, with perpendicular distance
        if (data.transform.y > 0 && stripe > 0 && stripe < SCREEN_WIDTH && data.transform.y < (renderer->ZBufferAt(stripe)))
        {
            for (int y = data.drawStart.y; y < data.drawEnd.y; y++) // for every pixel of the current stripe
            {
                int d = (y - data.screen.y) * 256 - SCREEN_HEIGHT * 128 + data.renderSize.y * 128; // 256 and 128 factors to avoid floats
                tex.y = ((d * m_surface->h()) / data.renderSize.y) / 256;
                
                Color color = m_surface->getColorAtPoint(tex.x, tex.y); // get current color from the texture

                if (!color)
                    continue;
                if (color == m_curSequence->GetMaskColor())
                    continue;
                if (color == m_curSequence->GetMaskColorAlt())
                    continue;
                if (m_curSequence->GetTexture()->m_clrKey && color == m_curSequence->GetTexture()->m_clrKey)
                    continue;

                ILightingSystem->ApplyLightForTile(tile, true, true, worldPos, stripe, y);

                renderer->SetPixel(stripe, y, color);
            }
        }
    }
}

void CAnimDirectional::OnCreate()
{
}

void CAnimDirectional::OnSequenceStart(const std::string &seq_name)
{
}

void CAnimDirectional::OnUpdate()
{
    m_curUpdate++;
    std::string seq_name = "";
    int frame_override = -1;
    m_orientation = DeduceOrientationCallback(&m_animflip, &m_animstate, &frame_override, seq_name);
   
    if(frame_override != -1){
        SwitchFrames(GetSequenceByLocalName(seq_name), (frame_override > 0) ? frame_override : 0); 
        m_playingForState = m_animstate;
         m_currentlyPlaying = seq_name; return;
    } 
    if(  m_currentlyPlaying != seq_name || m_playingForState != m_animstate)
    {
         m_playingForState = m_animstate; m_currentlyPlaying = seq_name;
         dbg("switching to %s", seq_name.c_str()); m_nextUpdate = 0; m_curFrame = 0;
        PlaySequenceByName(seq_name);   return;
    }
    
     
    if(m_curSequence == nullptr){
        assert(m_defaultSequence != nullptr); //bc i will forget to add one
        Error("m_curSeq null %s", m_currentlyPlaying.c_str() );
        BackToDefault();
        return;
    }

    if(m_nextUpdate > m_curUpdate) return;

    m_curFrame++;
    if(m_curFrame > m_curSequence->GetLastIndex()){
        OnSequenceEnd(m_curSequence->GetName());
        if(m_animstate == m_playingForState){
             m_nextUpdate = 0; m_curFrame = 0;
            PlaySequenceByName(seq_name);
             dbg("repeating %s", seq_name.c_str());
        }
             
        return;
    }
    
    SwitchFrames(m_curSequence, m_curFrame);
    m_nextUpdate = m_curUpdate + m_curSequence->GetRate();


    m_currentlyPlaying = seq_name;

}

void CAnimDirectional::OnSequenceEnd(const std::string &seq_name)
{
}

void CAnimDirectional::ChangeBaseTexture(const std::string &texture_name)
{
    
    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
    auto newTexture = ITextureSystem->FindOrCreatetexture(texture_name);
    for(auto& seq : m_seq)
    {
        seq.second->SetTexture(newTexture);
    }
    log("overrode texture to %s", texture_name.c_str());
}

uint32_t CAnimDirectional::GetPixelAtPoint(const IVector2 &point, IVector2 *textpos , const sprite_draw_data& data)
{
    log("hit check pt[%d %d] ds[%d %d]  de[%d %d] ", point.x, point.y, data.drawStart.x, data.drawStart.y, data.drawEnd.x, data.drawEnd.y);
    if (!(data.drawStart.x <= point.x && point.x <= data.drawEnd.x ) )
        return 0u;
    if (!(data.drawStart.y <= point.y && point.y <= data.drawEnd.y ) )
        return 0u;
  
    int stripe = point.x;
  
    IVector2 tex;
    tex.x = int(256 * (stripe - ((-data.renderSize.x / 2) + data.screen.x)) * m_surface->w() / data.renderSize.x) / 256;
        if(m_animflip == AnimDir_FlipH){
                    tex.x = m_surface->w() - tex.x; //tex.y = m_surface->h() - tex.y; 
        }
    log("first round");
    //***likely dont need any of the checks below***
    // conditions in the if are:
    // 1) it's in front of camera plane  2) it's on the screen (left) 3) it's on the screen (right) //REMOVED 4) ZBuffer, with perpendicular distance
    if (data.transform.y > 0 && stripe > 0 && stripe < SCREEN_WIDTH ) // REMOVED: && transform.y < (renderer->ZBufferAt(stripe)) we shouldnt need bc we will just hit a wall first
    {
        int y = point.y;
         int d = (y - data.screen.y) * 256 - SCREEN_HEIGHT * 128 + data.renderSize.y * 128; // 256 and 128 factors to avoid floats
        tex.y = ((d * m_surface->h()) / data.renderSize.y) / 256;
        
        Color color = m_surface->getColorAtPoint(tex.x, tex.y); // get current color from the texture

        if (!color)
            return 0u;
        if (color == m_curSequence->GetMaskColor())
             return 0u;
        if (color == m_curSequence->GetMaskColorAlt())
             return 0u;
        if (m_curSequence->GetTexture()->m_clrKey && color == m_curSequence->GetTexture()->m_clrKey)
             return 0u;
        log("HIT");
        return color;

       
    }
    

    return 0u;
}
