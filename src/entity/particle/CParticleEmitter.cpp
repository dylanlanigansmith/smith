#include "CParticleEmitter.hpp"
#include <util/misc.hpp>
#include <engine/engine.hpp>

REGISTER_DEF_ENT(CParticleEmitter);

/*
POC wip

*/

void CParticleEmitter::Render(CRenderer *renderer)
{
    draw.camera = renderer->GetActiveCamera();
    CalculateDrawInfo(draw);
   
         for(auto& pos : m_particles){
            if(draw.drawStart == m_lastDrawStart){
                if(IEngineTime->GetCurLoopTick() % 5 != 0)
                    break;
            }
            pos.x = Util::SemiRandRange(draw.drawStart.x, draw.drawEnd.x);
            pos.y = Util::SemiRandRange(draw.drawStart.y, draw.drawEnd.y);
        }
    
    m_lastDrawStart = draw.drawStart;
    int x_center =  (draw.drawEnd.x - draw.drawStart.x) / 2;
    int y_center =  (draw.drawEnd.y - draw.drawStart.y) / 2; //might be negative
    int amt = 0;
    Color part_color(233,123,32,210);
    int part_sizeX = 4;
    int part_sizeY = 4;
    auto tile = ILevelSystem->GetTileAtFast(m_vecPosition.x, m_vecPosition.y);
   
    for(auto& pos : m_particles){
        //if(!pos.x && !pos.y) continue;


        if (draw.transform.y > 0 && pos.x > 0 && pos.x < SCREEN_WIDTH - part_sizeX && draw.transform.y < (renderer->ZBufferAt(pos.x))){
            if( pos.y < 0 || pos.y > SCREEN_HEIGHT - part_sizeY) continue;
            if(renderer->Z2D[pos.x][pos.y] > 0.f &&  draw.transform.y > renderer->Z2D[pos.x][pos.y]   ) continue; //god damn it i guess we do it this way 



                Color bg_color = renderer->GetPixel(pos.x, pos.y);
                if(part_color.a() < 255)
                    bg_color = part_color + bg_color;
                

                ILightingSystem->ApplyLightForTile(tile, true, true, m_vecPosition, pos.x, pos.y);
                for(int i = 0; i < part_sizeX; i++){
                    for(int j = 0; j < part_sizeY; j++){
                        renderer->SetPixel(pos.x + i , pos.y + j, part_color);
                    }
                    
                }
                
                amt++;

        }
    }
   // gLog("parts %d {%d %d} {%d %d} %d x %d y", amt, draw.drawStart.x, draw.drawStart.y, draw.drawEnd.x, draw.drawEnd.y, x_center, y_center);
}

void CParticleEmitter::OnCreate()
{
    ENT_SETUP();
    draw.params.wScale = 3.0;
    draw.params.vScale = 2.0;

    draw.camera = IEntitySystem->GetLocalPlayer()->m_pCamera();
     CalculateDrawInfo(draw);
     m_lastDrawStart = draw.drawStart;
    for(auto& pos : m_particles){
        pos.x = Util::SemiRandRange(draw.drawStart.x, draw.drawEnd.x);
        pos.y = Util::SemiRandRange(draw.drawStart.y, draw.drawEnd.y);
    }
    
}

void CParticleEmitter::CreateRenderable()
{
    SetupSystem();
}

void CParticleEmitter::SetupSystem()
{
}