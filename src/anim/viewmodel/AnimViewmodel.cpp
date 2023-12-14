#include "AnimViewmodel.hpp"
#include <engine/engine.hpp>
#include <entity/player/CPlayer.hpp>
void CAnimViewmodel::Draw(CRenderer *renderer, const IVector2 &screen_pos, uint8_t alpha)
{
    static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
   

    if(m_curSequence == nullptr){
        Error("trying to draw with curSeq null %li", m_curUpdate); return;
    }
    if(auto& r = m_curSequence->GetFrames()[m_curFrame].m_rect; r.w == 0 && r.h == 0 ) return;


     auto worldPos =  m_parent->GetPosition();
    auto tile = ILevelSystem->GetTileAtFast(worldPos.x, worldPos.y);

    int start_x = SCREEN_WIDTH / 2 - m_surface->w() / 2 + screen_pos.x;
    int start_y = SCREEN_HEIGHT - m_surface->h() + screen_pos.y;
 
    for (int y = 0; y < m_surface->h(); ++y)
        for (int x = 0; x < m_surface->w(); ++x)
        {
            Color color = m_surface->getColorAtPoint(x,y);
            if(x + start_x >= SCREEN_WIDTH) continue;
            if(y + start_y >= SCREEN_HEIGHT) continue;
            if (!color)
                continue;
            if (color == m_curSequence->GetMaskColor())
                continue;
            if (color == m_curSequence->GetMaskColorAlt())
                continue;
             if (m_curSequence->GetTexture()->m_clrKey && color == m_curSequence->GetTexture()->m_clrKey )
                continue;
             //ILightingSystem->ApplyLightForTile(tile, true, true, worldPos, start_x + x, start_y + y); <- looks like shit
            if(alpha == 255)
                renderer->SetPixel(start_x + x, start_y + y, color);
            else{
                color.a(alpha);
                auto behind = renderer->GetPixel(start_x + x, start_y + y);
             
                
                 renderer->SetPixel(start_x + x, start_y + y, color + behind); //bug?
            }
            
        }
}

void CAnimViewmodel::OnSequenceStart(const std::string &seq_name)
{
   
}

void CAnimViewmodel::OnSequenceEnd(const std::string &seq_name)
{

}
