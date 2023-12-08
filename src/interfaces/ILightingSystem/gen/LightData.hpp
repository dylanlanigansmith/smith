#pragma once
#include <engine/engine.hpp>
#include <interfaces/ILightingSystem/ILightingSystem.hpp>
#include <types/Vector.hpp>



class LightData
{
public:
    LightData(CLightingSystem* ILightingSystem) : ILightingSystem(ILightingSystem) {
        ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    }
    void DebugPerPixel()
    {
        auto& world = ILevelSystem->m_Level->world;

        for( auto& row : world ) for( auto& tile : row){

            Vector tile_pos = {tile.m_vecPosition.x, tile.m_vecPosition.y, 0.f};
            if(tile.isEmpty()) continue;
            auto tile_type = tile.m_nType;
            for (int x = 0; x < TILE_SECTORS; ++x)
                for (int y = 0; y < TILE_SECTORS; ++y)
                    for (int z = 0; z < TILE_SECTORS; ++z)
                    {
                        if(tile_type == Level::Tile_Wall && x == 1 && y == 1) continue; //center of wall is dark
                        auto pos = tile.getSectorCenterRelativeCoords(x,y,z);
                        pos = tile_pos + pos;

                        Color total_color = ILightingSystem->MaxDark();
                        auto voxel = tile.getVoxelAt(x,y,z);
                        voxel->m_light = total_color;
                        std::vector<CLight*> influential_lights;
                        for(auto& entry : ILightingSystem->light_list){
                            auto light = entry.second;
                            if((pos - light->GetPosition() ).Length3D() <= light->GetRange() )
                            {
                                influential_lights.push_back(light);
                            }
                        }
                        Color d = Color::White();
                       // voxel->m_light = Color(d[0] + d[1], 0,d[3] + d[2],0);
                       
                        if(x == 0)
                            voxel->m_light = Color::Red();
                        if(x == 0 && y == 1 && z == 0)
                            voxel->m_light = Color::Green();
                        if(x == 2)
                            voxel->m_light = Color::Blue();
                        if(influential_lights.empty()) continue;
                        
                        
                        
                        //if(z == 2)
                          //  voxel->m_light = voxel->m_light + Color::SkyBlue();
                        for(auto& light : influential_lights)
                        {
                            Ray_t ray = {
                                .origin = Vector2(pos),
                                .direction = Vector2( Vector(pos - light->GetPosition()).Normalize() )
                            };

                        }       
                        


                    }
        }   
    }
    
    inline Color CalcColor(CLight* light, Color total_light, float distance){
        auto& params = ILightingSystem->params;
        float attenuation = 1.0f / (1.0f + ILightingSystem->params.a * distance + params.b * distance * distance);
        attenuation = 1.f - std::clamp(attenuation, params.minIntensity, 1.0f) * params.finalAlphaMod;

        Color lightColor = light->GetColor();
        float brightness =  (1.f - light->GetIntensity()) * attenuation;
        float alphaFactor = ( brightness) * params.brightFactorMod;
        lightColor.a(static_cast<uint8_t>(ILightingSystem->MaxDark().a() * alphaFactor));

        if(total_light.a() == ILightingSystem->MaxDark().a())
            total_light.a(lightColor.a());
        return lightColor + total_light;

    }
    void Calculate()
    {
        auto& world = ILevelSystem->m_Level->world;

        for( auto& row : world ) for( auto& tile : row){

            Vector tile_pos = {tile.m_vecPosition.x, tile.m_vecPosition.y, 0.f};
            auto tile_type = tile.m_nType;
            for (int x = 0; x < TILE_SECTORS; ++x)
                for (int y = 0; y < TILE_SECTORS; ++y)
                    for (int z = 0; z < TILE_SECTORS; ++z)
                    {
                        Color total_color = ILightingSystem->MaxDark();
                        auto voxel = tile.getVoxelAt(x,y,z);
                        voxel->m_light = total_color;
                        if(tile_type == Level::Tile_Wall && x == 1 && y == 1) continue; //center of wall is dark
                        auto pos = tile.getSectorCenterRelativeCoords(x,y,z);
                        pos = tile_pos + pos;


                       // ILightingSystem->log("World Pos For [%i %i %i] == {%.4f %.4f %.4f} for tile [%.1f %.1f]", x,y,z, pos.x, pos.y,pos.z, tile_pos.x, tile_pos.y);
                        
                        std::vector<CLight*> influential_lights;
                        for(auto& entry : ILightingSystem->light_list){
                            auto light = entry.second;
                            if((pos - light->GetPosition() ).Length3D() <= light->GetRange() )
                            {
                                influential_lights.push_back(light);
                            }
                        }
                        if(influential_lights.empty()) continue;

                        for(auto& light : influential_lights)
                        {
                            Ray_t ray = {
                                .origin = Vector2(light->GetPosition()),
                                .direction = Vector2( Vector(pos - light->GetPosition()).Normalize() )
                            };
                            
                            int hit = 0;
                            const double step = 0.16;
                            Vector2 light_pos = Vector2( light->GetPosition());
                            Vector2 rayPos = ray.origin;
                            const double dist_to_point = (Vector2(pos) - light_pos).Length();
                            auto light_tile = ILevelSystem->GetTileAt(light_pos) ;
                            IVector2 last_tile_pos = light_tile->m_vecPosition; 
                            int walls = 0;
                            int iterations = 0;
                            int expected_iterations = dist_to_point / step + 1;
                            
                            while(1)
                            {
                                iterations++;
                                rayPos = rayPos + (ray.direction * step);
                                if(floor(rayPos.x) == last_tile_pos.x && floor(rayPos.y) == last_tile_pos.y && last_tile_pos != tile.m_vecPosition) continue;
                                auto ray_tile = ILevelSystem->GetTileAt(rayPos.x, rayPos.y);
                              

                                if(!ray_tile ) break; //|| rayPos.x < 0.0 || rayPos.y < 0.0 || rayPos.x > MAP_SIZE || rayPos.y > MAP_SIZE
                                last_tile_pos = ray_tile->m_vecPosition;
                                 double rayDist = (Vector2(pos) - rayPos).Length();
                              //  if(rayDist >  1.5 * light->GetRange()) break;
                               // if(rayDist > dist_to_point) break;
                                if(ray_tile->m_nType == Level::Tile_Wall)
                                {
                                    walls++;
                                    if(ray_tile->m_vecPosition != tile.m_vecPosition || walls > 2){
                                        ILightingSystem->log("Ray End Pos For Tile [%.1f %.1f] and goal {%.4f %.4f %.4f}: Hit Wall {%i %i} Ray Dist %f Total Dist %f",  
                                                tile_pos.x, tile_pos.y, pos.x, pos.y,pos.z, last_tile_pos.x, last_tile_pos.y, rayDist, dist_to_point);
                                        break;
                                    }
                                        
                                }
                                 if(tile_type == Level::Tile_Empty || (rayDist < (step * 2))){
                                    if(last_tile_pos == tile.m_vecPosition || last_tile_pos == light_tile->m_vecPosition){
                                        total_color = CalcColor(light, total_color, dist_to_point); break;
                                         ILightingSystem->log("Ray End Pos For Tile [%.1f %.1f] and goal {%.4f %.4f %.4f}: Success",  tile_pos.x, tile_pos.y, pos.x, pos.y,pos.z);
                                    }

                                }
                                 if( (rayDist < (step * 2)) && (last_tile_pos == tile.m_vecPosition ) )
                                {
                                    ILightingSystem->log("Ray End Pos For Tile [%.1f %.1f] and goal {%.4f %.4f %.4f}: Success",  tile_pos.x, tile_pos.y, pos.x, pos.y,pos.z);
                                    total_color = CalcColor(light, total_color, dist_to_point); break;
                                }
                               
                                
                                
                            }

                        }

                        voxel->m_light = total_color;       
                
                    }
        }   
    }


private:
    CLightingSystem* ILightingSystem;
    CLevelSystem* ILevelSystem;
};