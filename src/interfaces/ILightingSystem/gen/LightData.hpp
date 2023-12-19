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
        float attenuation = 1.0f / (1.0f + params.a * distance + params.b * distance * distance);
        attenuation = (1.f - std::clamp(attenuation, params.minIntensity, 1.0f)) * params.rolloff;

        Color lightColor = light->GetColor();
        float brightness =  (1.f - light->GetIntensity()) * attenuation;
        float alphaFactor = ( brightness) * params.intensityModifier;
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
                            if((pos - light->GetPosition() ).Length3D() <= light->GetRange() ) //length 2d vs 3d makes a huge difference
                            {
                                influential_lights.push_back(light);
                            }
                        }
                        if(influential_lights.empty()) continue;
                     
                       
                        for(auto& light : influential_lights)
                        {
                            auto find = std::find(tile.influential_lights.begin(), tile.influential_lights.end(), light);
                             if(find == tile.influential_lights.end()  ){
                                tile.influential_lights.push_back(light);
                             }
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
                             auto& params = ILightingSystem->params;
                            while(1)
                            {
                                iterations++;
                                rayPos = rayPos + (ray.direction * step);
                                if(floor(rayPos.x) == last_tile_pos.x && floor(rayPos.y) == last_tile_pos.y && last_tile_pos != tile.m_vecPosition) continue;
                                auto ray_tile = ILevelSystem->GetTileAt(rayPos.x, rayPos.y);
                              

                                if(!ray_tile ) break; 
                                last_tile_pos = ray_tile->m_vecPosition;
                                 double rayDist = (Vector2(pos) - rayPos).Length();
                             
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
                                        total_color = light->CalculateInfluence(pos, total_color, params, ILightingSystem->MaxDark()); break;
                                        ILightingSystem->log("Ray End Pos For Tile [%.1f %.1f] and goal {%.4f %.4f %.4f}: Success",  tile_pos.x, tile_pos.y, pos.x, pos.y,pos.z);
                                    }

                                }
                                 if( (rayDist < (step * 2)) && (last_tile_pos == tile.m_vecPosition ) )
                                {
                                    ILightingSystem->log("Ray End Pos For Tile [%.1f %.1f] and goal {%.4f %.4f %.4f}: Success",  tile_pos.x, tile_pos.y, pos.x, pos.y,pos.z);
                                    total_color = light->CalculateInfluence(pos, total_color, params, ILightingSystem->MaxDark()); break;
                                }
                               
                                
                                
                            }

                        }

                        voxel->m_light = total_color;       
                
                    }
        }   
    }

/*
The real thing needed:
a better light model and attenuation

also the code below f-ing sucks 
like its so bad
when it breaks again
please dont add another patch
please fix it
I beg 
I plead
12/18/23

*/


    void Calculate2()
    {
         //  Vector tile_pos = {tile.m_vecPosition.x, tile.m_vecPosition.y, 0.f};
        /*
            for (float x = 0.f; x < MAP_SIZE ; x += 0.1f)
                for (float y = 0.f; y < MAP_SIZE; y += 0.1f)
                    for (float z = 0.f; z < 1.0; z += 0.1f){
                         int xIndex = std::clamp(static_cast<int>( std::round(x * 10.f)), 0, MAP_SIZE * 10);
                        int yIndex = std::clamp(static_cast<int>(std::round(y * 10.f)), 0, MAP_SIZE * 10);
                        int zIndex = std::clamp(static_cast<int>(std::round(z * 10.f)), 0, 1 * 10);
                       
                        //
                        ILightingSystem->lightmap[xIndex][yIndex][zIndex] = Color(255,0,0,255);
                    }
*/
            for(auto& a0 : ILightingSystem->lightmap){
                for(auto& a1 : a0){
                    for(auto& a2: a1)
                        a2 = Color(255,0,0,255); //for finding bugs easily
                }
            }
            for(auto& entry : ILightingSystem->light_list){
                if(!entry.second->rays.empty())
                    entry.second->rays.clear();
            }
                         



            for (double x = 0.f; x < MAP_SIZE ; x += 0.1f)
                for (double y = 0.f; y < MAP_SIZE; y += 0.1f)
                    for (double z = 0.f; z < 1.0; z += 0.1f)
                    {
                        

                          int xIndex = std::clamp(static_cast<int>( std::round(x * 10.f)), 0, MAP_SIZE * 10);
                        int yIndex = std::clamp(static_cast<int>(std::round(y * 10.f)), 0, MAP_SIZE * 10);
                        int zIndex = std::clamp(static_cast<int>(std::round(z * 10.f)), 0, 1 * 10);
                        auto tile = ILevelSystem->GetTileAt(floor(x),floor(y));
                        Color total_color = ILightingSystem->MaxDark();
                         ILightingSystem->lightmap[xIndex][yIndex][zIndex] = total_color;
                       
                        if(!tile){
                           
                            
                            ILevelSystem->log("no tile at %f %f %f", x,y,z);
                            continue;
                        }
                        auto tile_type = tile->m_nType;
                        Vector pos = {x,y,z};
                      
                     

                        
                          
                        // ILightingSystem->log("START FOR [%.1f %.1f %.1f]", x,y,z);
                     //   ILightingSystem->log("Pos [%.1f %.1f %.1f][%f %f %f] ==  Arr{%d %d %d} at tile [%d %d]", x,y,z, pos.x, pos.y,pos.z, xIndex, yIndex, zIndex, tile->m_vecPosition.x, tile->m_vecPosition.y);
                        
                        std::vector<CLight*> influential_lights;
                        for(auto& entry : ILightingSystem->light_list){

                            auto light = entry.second;
                             auto p = light->GetPosition();
                            
                            if((pos - light->GetPosition() ).Length2D() <= light->GetRange() ) //length 2d vs 3d makes a huge difference
                            {
                            //    ILightingSystem->log("trying light: %s {%.1f %.1f %.1f } | %s", light->GetName().c_str(), p.x, p.y, p.z, light->GetColor().s().c_str());
                            //   ILightingSystem->log("^^^ IS INFLUENCE");
                                influential_lights.push_back(light);
                                continue;
                            }
                          //   ILightingSystem->log("not an influence: %f dist  %f range", (pos - light->GetPosition() ).Length3D(), light->GetRange() );

                        }

//                        ILightingSystem->tested_points.erase(std::remove(ILightingSystem->tested_points.begin(), ILightingSystem->tested_points.end(), pos), ILightingSystem->tested_points.end());
                        if(influential_lights.empty()){
                           

                          
                          //  ILightingSystem->log("no influential lights at %f %f", x,y);
                        //  ILightingSystem->log("END FOR [%.1f %.1f %.1f] \n", x,y,z);

                            continue;
                        }
                        bool logpt = false;
                        if((pos.z <= 0.2 || pos.z >= 0.9) && pos.x < 15.1 && pos.x > 14.9 && pos.y < 21.1 && pos.y > 20.9){
                            logpt = true;
                        }
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
                             auto& params = ILightingSystem->params;
                            
                             const double epsilon = 1e-6; // or another small value
                            bool isWhole = (std::abs(pos.x - floor(pos.x)) < epsilon || std::abs(pos.y - floor(pos.y)) < epsilon);
                            while(1)
                            {
                                iterations++;
                                rayPos = rayPos + (ray.direction * step);
                                if(logpt)
                                    ILightingSystem->log(" %d RAY  {%.4f %.4f } TO {%.4f %.4f %.4f} t[%d %d] last[%d %d]  walls%d", iterations, rayPos.x, rayPos.y,  pos.x, pos.y,pos.z, last_tile_pos.x, last_tile_pos.y,
                                            tile->m_vecPosition.x, tile->m_vecPosition.y, walls );
                                 double rayDist = (Vector2(pos) - rayPos).Length();
                                if(floor(rayPos.x) == last_tile_pos.x && floor(rayPos.y) == last_tile_pos.y && last_tile_pos != tile->m_vecPosition &&  (rayDist >= (step * 2)  )) continue;
                                auto ray_tile = ILevelSystem->GetTileAt(rayPos.x, rayPos.y);
                                if(logpt)
                                    ILightingSystem->log(" %d DID ASSESS [%d %d]",iterations,  ray_tile->m_vecPosition.x, ray_tile->m_vecPosition.y);

                                if(!ray_tile ) break; 
                                last_tile_pos = ray_tile->m_vecPosition;
                                

                                if(ray_tile->m_nType == Level::Tile_Wall)
                                {
                                    
                                    walls++;
                                    if(ray_tile->m_vecPosition != tile->m_vecPosition || walls > 2){
                                        if(logpt){
                                            light->rays.push_back({rayPos, pos, false});
                                            ILightingSystem->log("RAY  {%.4f %.4f } MISS FOR   {%.4f %.4f %.4f} t[%d %d] rt[%d %d]  %s  %d",rayPos.x, rayPos.y, pos.x, pos.y,pos.z,
                                            tile->m_vecPosition.x, tile->m_vecPosition.y, ray_tile->m_vecPosition.x, ray_tile->m_vecPosition.y, (total_color == ILightingSystem->MaxDark()) ? "MAXDARK" : "HASLIGHT", walls );
                                        }
                                            
                                        break;
                                    }
                                        
                                }
                                 if(tile_type == Level::Tile_Empty || (rayDist < (step * 2))){
                                   

                                    if(last_tile_pos == tile->m_vecPosition || last_tile_pos == light_tile->m_vecPosition || (isWhole && IVector2::Rounded(pos) == IVector2::Rounded(rayPos))){
                                        if(logpt)
                                            ILightingSystem->log(" %d MADE IT IN <ray dist %f> step*2 <%f>  ltp [%d %d] p roudned [%d %d] ",iterations, rayDist, (step * 2), last_tile_pos.x, last_tile_pos.y, IVector2::Rounded(pos).x, IVector2::Rounded(pos).y  );
                                        total_color = light->CalculateInfluence(pos, total_color, params, ILightingSystem->MaxDark());
                                         if(total_color.r() == 45 && total_color.a() == 0){
                                            //  ILightingSystem->log("{%.4f %.4f %.4f}   idx[%d %d %d]  total_color:  %s ",pos.x, pos.y,pos.z,xIndex, yIndex, zIndex, total_color.s().c_str());
                                             //  auto p = light->GetPosition();
                                            //  ILightingSystem->log("light: %s {%.1f %.1f %.1f } |color: %s", light->GetName().c_str(), p.x, p.y, p.z, light->GetColor().s().c_str());
                                             //  ILightingSystem->log(" pos->light %f dist light %f range", (pos - light->GetPosition() ).Length3D(), light->GetRange() );
                                         }
                                         if(logpt){
                                            light->rays.push_back({rayPos, pos, true});
                                            ILightingSystem->log("RAY HIT (empty or dist) FOR   {%.4f %.4f %.4f} [%d %d %d]  %s ",pos.x, pos.y,pos.z,xIndex, yIndex, zIndex, (total_color == ILightingSystem->MaxDark()) ? "MAXDARK" : "HASLIGHT" );
                                         }
                                            
                                        break;
                                        
                                    }

                                }
                              
                                 if( (rayDist < (step * 2)) &&  ((last_tile_pos == tile->m_vecPosition ) || (isWhole && IVector2::Rounded(pos) == IVector2::Rounded(rayPos))) )
                                {
                                  
                                    total_color = light->CalculateInfluence(pos, total_color, params, ILightingSystem->MaxDark()); 
                                    if(logpt){
                                            light->rays.push_back({rayPos, pos, true});
                                            ILightingSystem->log("RAY HIT (wall) FOR   {%.4f %.4f %.4f} [%d %d %d]  %s ",pos.x, pos.y,pos.z,xIndex, yIndex, zIndex, (total_color == ILightingSystem->MaxDark()) ? "MAXDARK" : "HASLIGHT" );
                                         }
                                    break;
                                }
                               
                                 if(logpt)
                                    ILightingSystem->log(" %d END ASSESS <ray dist %f> step*2 <%f> %d is whole",iterations, rayDist, (step * 2), isWhole );
                                
                            }

                        }

                       
                      if(logpt){
                      //  if(pos.x < 9.1f && pos.x < 9.6f )
                             ILightingSystem->log(" end pos{%.4f %.4f %.4f}   %d %d %d  %s ",pos.x, pos.y,pos.z,xIndex, yIndex, zIndex, (total_color == ILightingSystem->MaxDark()) ? "MAX DARK" : total_color.s().c_str());
                      }
                      
                      
                        ILightingSystem->lightmap[xIndex][yIndex][zIndex] = total_color;
                      //  ILightingSystem->log("final color %s %s", total_color.s().c_str(), (total_color == ILightingSystem->MaxDark()) ? "MAXDARK" : "HASLIGHT" );
                       // ILightingSystem->log("END FOR [%.1f %.1f %.1f]", x,y,z);
                    }
    }   
    
private:
    CLightingSystem* ILightingSystem;
    CLevelSystem* ILevelSystem;
};