#pragma once
#include <engine/engine.hpp>
#include <interfaces/ILightingSystem/ILightingSystem.hpp>
#include <types/Vector.hpp>

//https://www.google.com/search?q=cosine+term+lighting&oq=cosine+term+lighting&gs_lcrp=EgZjaHJvbWUyBggAEEUYOTIHCAEQIRigAdIBCDIzNDVqMGo3qAIAsAIA&sourceid=chrome&ie=UTF-8

class LightData //not a namespace bc it needs friend access for CLevel
{
public:
    //these 3 are old
    static void DebugPerPixel()
    {
        auto &world = ILevelSystem->m_Level->world;

        for (auto &row : world)
            for (auto &tile : row)
            {

                Vector tile_pos = {tile.m_vecPosition.x, tile.m_vecPosition.y, 0.f};
                if (tile.isEmpty())
                    continue;
                auto tile_type = tile.m_nType;
                for (int x = 0; x < TILE_SECTORS; ++x)
                    for (int y = 0; y < TILE_SECTORS; ++y)
                        for (int z = 0; z < TILE_SECTORS; ++z)
                        {
                            if (tile_type == Level::Tile_Wall && x == 1 && y == 1)
                                continue; // center of wall is dark
                            auto pos = tile.getSectorCenterRelativeCoords(x, y, z);
                            pos = tile_pos + pos;

                            Color total_color = ILightingSystem->MaxDark();
                            auto voxel = tile.getVoxelAt(x, y, z);
                            voxel->m_light = total_color;
                            std::vector<CLight *> influential_lights;
                            for (auto &entry : ILightingSystem->light_list)
                            {
                                auto light = entry.second;
                                if ((pos - light->GetPosition()).Length3D() <= light->GetRange())
                                {
                                    influential_lights.push_back(light);
                                }
                            }
                            Color d = Color::White();
                            // voxel->m_light = Color(d[0] + d[1], 0,d[3] + d[2],0);

                            if (x == 0)
                                voxel->m_light = Color::Red();
                            if (x == 0 && y == 1 && z == 0)
                                voxel->m_light = Color::Green();
                            if (x == 2)
                                voxel->m_light = Color::Blue();
                            if (influential_lights.empty())
                                continue;

                            // if(z == 2)
                            //   voxel->m_light = voxel->m_light + Color::SkyBlue();
                            for (auto &light : influential_lights)
                            {
                                Ray_t ray = {
                                    .origin = Vector2(pos),
                                    .direction = Vector2(Vector(pos - light->GetPosition()).Normalize())};
                            }
                        }
            }
    }

    inline Color CalcColor(CLight *light, Color total_light, float distance)
    {
        auto &params = ILightingSystem->params;
        float attenuation = 1.0f / (1.0f + params.a * distance + params.b * distance * distance);
        attenuation = (1.f - std::clamp(attenuation, params.minIntensity, 1.0f)) * params.rolloff;

        Color lightColor = light->GetColor();
        float brightness = (1.f - light->GetIntensity()) * attenuation;
        float alphaFactor = (brightness)*params.intensityModifier;
        lightColor.a(static_cast<uint8_t>(ILightingSystem->MaxDark().a() * alphaFactor));

        if (total_light.a() == ILightingSystem->MaxDark().a())
            total_light.a(lightColor.a());
        return lightColor + total_light;
    }
    static void Calculate()
    {
        auto &world = ILevelSystem->m_Level->world;

        for (auto &row : world)
            for (auto &tile : row)
            {

                Vector tile_pos = {tile.m_vecPosition.x, tile.m_vecPosition.y, 0.f};
                auto tile_type = tile.m_nType;
                for (int x = 0; x < TILE_SECTORS; ++x)
                    for (int y = 0; y < TILE_SECTORS; ++y)
                        for (int z = 0; z < TILE_SECTORS; ++z)
                        {
                            Color total_color = ILightingSystem->MaxDark();
                            auto voxel = tile.getVoxelAt(x, y, z);
                            voxel->m_light = total_color;
                            if (tile_type == Level::Tile_Wall && x == 1 && y == 1)
                                continue; // center of wall is dark
                            auto pos = tile.getSectorCenterRelativeCoords(x, y, z);
                            pos = tile_pos + pos;

                            // ILightingSystem->log("World Pos For [%i %i %i] == {%.4f %.4f %.4f} for tile [%.1f %.1f]", x,y,z, pos.x, pos.y,pos.z, tile_pos.x, tile_pos.y);

                            std::vector<CLight *> influential_lights;
                            for (auto &entry : ILightingSystem->light_list)
                            {
                                auto light = entry.second;
                                if ((pos - light->GetPosition()).Length3D() <= light->GetRange()) // length 2d vs 3d makes a huge difference
                                {
                                    influential_lights.push_back(light);
                                }
                            }
                            if (influential_lights.empty())
                                continue;

                            for (auto &light : influential_lights)
                            {

                                Ray_t ray = {
                                    .origin = Vector2(light->GetPosition()),
                                    .direction = Vector2(Vector(pos - light->GetPosition()).Normalize())};

                                int hit = 0;
                                const double step = 0.16;
                                Vector2 light_pos = Vector2(light->GetPosition());
                                Vector2 rayPos = ray.origin;
                                const double dist_to_point = (Vector2(pos) - light_pos).Length();
                                auto light_tile = ILevelSystem->GetTileAt(light_pos);
                                IVector2 last_tile_pos = light_tile->m_vecPosition;
                                int walls = 0;
                                int iterations = 0;
                                int expected_iterations = dist_to_point / step + 1;
                                auto &params = ILightingSystem->params;
                                while (1)
                                {
                                    iterations++;
                                    rayPos = rayPos + (ray.direction * step);
                                    if (floor(rayPos.x) == last_tile_pos.x && floor(rayPos.y) == last_tile_pos.y && last_tile_pos != tile.m_vecPosition)
                                        continue;
                                    auto ray_tile = ILevelSystem->GetTileAt(rayPos.x, rayPos.y);

                                    if (!ray_tile)
                                        break;
                                    last_tile_pos = ray_tile->m_vecPosition;
                                    double rayDist = (Vector2(pos) - rayPos).Length();

                                    if (ray_tile->m_nType == Level::Tile_Wall)
                                    {
                                        walls++;
                                        if (ray_tile->m_vecPosition != tile.m_vecPosition || walls > 2)
                                        {
                                            ILightingSystem->log("Ray End Pos For Tile [%.1f %.1f] and goal {%.4f %.4f %.4f}: Hit Wall {%i %i} Ray Dist %f Total Dist %f",
                                                                 tile_pos.x, tile_pos.y, pos.x, pos.y, pos.z, last_tile_pos.x, last_tile_pos.y, rayDist, dist_to_point);
                                            break;
                                        }
                                    }
                                    if (tile_type == Level::Tile_Empty || (rayDist < (step * 2)))
                                    {
                                        if (last_tile_pos == tile.m_vecPosition || last_tile_pos == light_tile->m_vecPosition)
                                        {
                                            total_color = light->CalculateInfluence(pos, total_color, params, ILightingSystem->MaxDark());
                                            break;
                                            ILightingSystem->log("Ray End Pos For Tile [%.1f %.1f] and goal {%.4f %.4f %.4f}: Success", tile_pos.x, tile_pos.y, pos.x, pos.y, pos.z);
                                        }
                                    }
                                    if ((rayDist < (step * 2)) && (last_tile_pos == tile.m_vecPosition))
                                    {
                                        ILightingSystem->log("Ray End Pos For Tile [%.1f %.1f] and goal {%.4f %.4f %.4f}: Success", tile_pos.x, tile_pos.y, pos.x, pos.y, pos.z);
                                        total_color = light->CalculateInfluence(pos, total_color, params, ILightingSystem->MaxDark());
                                        break;
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

    12/26/23 - patched fix for dynamic changes

    */
    static inline ivec3 WorldToMapIndex(const Vector& pos){
        return {
            std::clamp(static_cast<int>(std::round(pos.x * 10.f)), 0, MAP_SIZE * 10),
            std::clamp(static_cast<int>(std::round(pos.y * 10.f)), 0, MAP_SIZE * 10),
            std::clamp(static_cast<int>(std::round(pos.z * 10.f)), 0, 1 * 10)
        };
    }
    static inline void UpdateLightForPoint(double x, double y, double z, CLight* override_light = nullptr)
    {
        int xIndex = std::clamp(static_cast<int>(std::round(x * 10.f)), 0, MAP_SIZE * 10);
        int yIndex = std::clamp(static_cast<int>(std::round(y * 10.f)), 0, MAP_SIZE * 10);
        int zIndex = std::clamp(static_cast<int>(std::round(z * 10.f)), 0, 1 * 10);
        auto tile = ILevelSystem->GetTileAt(floor(x), floor(y));
        Color total_color = ILightingSystem->MaxDark();
        ILightingSystem->lightmap[xIndex][yIndex][zIndex] = total_color;

        if (!tile)
        {
            ILevelSystem->log("no tile at %f %f %f", x, y, z);
            return;
        }
        auto tile_type = tile->m_nType;
        Vector pos = {x, y, z};

        // ILightingSystem->log("START FOR [%.1f %.1f %.1f]", x,y,z);
        //   ILightingSystem->log("Pos [%.1f %.1f %.1f][%f %f %f] ==  Arr{%d %d %d} at tile [%d %d]", x,y,z, pos.x, pos.y,pos.z, xIndex, yIndex, zIndex, tile->m_vecPosition.x, tile->m_vecPosition.y);
        
        std::vector<CLight *> influential_lights;
        if(override_light == nullptr)
            for (auto &entry : ILightingSystem->light_list)
            {

                auto light = entry.second;
                //if(light->IsTemporary()) continue;
                auto p = light->GetPosition();

                if ((pos - light->GetPosition()).Length2D() <= light->GetRange()) // length 2d vs 3d makes a huge difference
                {
                    influential_lights.push_back(light);
                    continue;
                }

            }
        else influential_lights.push_back(override_light);

        if (influential_lights.empty())
            return;
       
        for (auto &light : influential_lights)
        {

            Ray_t ray = {
                .origin = Vector2(light->GetPosition()),
                .direction = Vector2(Vector(pos - light->GetPosition()).Normalize())};

            int hit = 0;
            const double step = 0.16;
            Vector2 light_pos = Vector2(light->GetPosition());
            Vector2 rayPos = ray.origin;
            const double dist_to_point = (Vector2(pos) - light_pos).Length();
            auto light_tile = ILevelSystem->GetTileAt(light_pos);
            IVector2 last_tile_pos = light_tile->m_vecPosition;
            int walls = 0;
            int iterations = 0;
            int expected_iterations = dist_to_point / step + 1;
            auto &params = ILightingSystem->params;

            const double epsilon = 1e-6; // or another small value
            bool isWhole = (std::abs(pos.x - floor(pos.x)) < epsilon || std::abs(pos.y - floor(pos.y)) < epsilon);
            while (1)
            {
                iterations++;
                rayPos = rayPos + (ray.direction * step);
                
                                       
                double rayDist = (Vector2(pos) - rayPos).Length();
                if (floor(rayPos.x) == last_tile_pos.x && floor(rayPos.y) == last_tile_pos.y && last_tile_pos != tile->m_vecPosition && (rayDist >= (step * 2)))
                    continue;
                auto ray_tile = ILevelSystem->GetTileAt(rayPos.x, rayPos.y);
                

                if (!ray_tile)
                    break;
                last_tile_pos = ray_tile->m_vecPosition;
                if (ray_tile->IsThinWall() && !ray_tile->m_pTexture->isTransparent())
                {
                    if (ray_tile->m_flDoor < 0.9f)
                    {
                        auto wall = Render::GetLineForWallType(ray_tile->m_vecPosition, ray_tile->m_nType);
                        Vector2 intersection;
                        if (Util::RayIntersectsLineSegment(ray, wall, intersection))
                        {
                            light->rays.push_back({intersection, pos, false});
                            if(ray_tile->HasState() && override_light == nullptr){
                                ray_tile->m_pState->light_pts.push_back({pos, light});
                            }
                            break;
                        }
                    }
                }

                if (ray_tile->m_nType == Level::Tile_Wall)
                {

                    walls++;
                    if (ray_tile->m_vecPosition != tile->m_vecPosition || walls > 2)
                    {
                        if(override_light == nullptr)
                            light->rays.push_back({rayPos, pos, false});
                           

                        break;
                    }
                }
                if (tile_type == Level::Tile_Empty || (rayDist < (step * 2)))
                {

                    if (last_tile_pos == tile->m_vecPosition || last_tile_pos == light_tile->m_vecPosition || (isWhole && IVector2::Rounded(pos) == IVector2::Rounded(rayPos)))
                    {
                      
                        total_color = light->CalculateInfluence(pos, total_color, params, ILightingSystem->MaxDark());
                        if(override_light == nullptr && !light->IsTemporary()){ //aka we are doing initial generation
                            light->AddInfluence(pos);
                            light->rays.push_back({rayPos, pos, true});
                        }
                                
                        
                        
                          

                        break;
                    }
                }

                if ((rayDist < (step * 2)) && ((last_tile_pos == tile->m_vecPosition) || (isWhole && IVector2::Rounded(pos) == IVector2::Rounded(rayPos))))
                {

                    total_color = light->CalculateInfluence(pos, total_color, params, ILightingSystem->MaxDark());
                     if(override_light == nullptr && !light->IsTemporary()){
                        light->AddInfluence(pos);
                        light->rays.push_back({rayPos, pos, true});
                       }
                      
                    break;
                }

                
            }
        }

        

        ILightingSystem->lightmap[xIndex][yIndex][zIndex] = total_color;
        //  ILightingSystem->log("final color %s %s", total_color.s().c_str(), (total_color == ILightingSystem->MaxDark()) ? "MAXDARK" : "HASLIGHT" );
        // ILightingSystem->log("END FOR [%.1f %.1f %.1f]", x,y,z);
    }

    static void Calculate2()
    {

        for (auto &a0 : ILightingSystem->lightmap)
        {
            for (auto &a1 : a0)
            {
                for (auto &a2 : a1)
                    a2 = Color(255, 0, 0, 255); // for finding bugs easily
            }
        }
        for (auto &entry : ILightingSystem->light_list)
        {
            if (!entry.second->rays.empty())
                entry.second->rays.clear();
            entry.second->Reset();
        }

        for (double x = 0.f; x < MAP_SIZE; x += 0.1f)
            for (double y = 0.f; y < MAP_SIZE; y += 0.1f)
                for (double z = 0.f; z < 1.0; z += 0.1f)
                {

                    UpdateLightForPoint(x, y, z);
                }
    }

private:
    
};