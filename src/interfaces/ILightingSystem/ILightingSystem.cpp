#include "ILightingSystem.hpp"
#include <interfaces/ILevelSystem/ILevelSystem.hpp>
#include <engine/engine.hpp>

Color CLightingSystem::CalculateLightInfluence(CLight *light, const Vector &point)
{

    return Color::None();
}

void CLightingSystem::OnEngineInitFinish()
{
    Debug(false);
}

void CLightingSystem::SetupBlending()
{
    // SDL_SetTextureBlendMode(m_lighttexture, SDL_BLENDMODE_BLEND);
}

void CLightingSystem::RegenerateLighting()
{
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    auto &level = ILevelSystem->m_Level;
    auto &world = level->world;
    int lol = 0;
    voxel_t empty{};
    for (auto &row : world)
    {
        for (auto &tile : row)
        {
            for (int x = 0; x < TILE_SECTORS; ++x)
                for (int y = 0; y < TILE_SECTORS; ++y)
                    for (int z = 0; z < TILE_SECTORS; ++z)
                    {
                        tile.sectors[x][y][z] = empty;
                        lol++;
                    }
        }
    }
    log("cleared color info for %i faces", lol);
    CalculateLighting();
}

void CLightingSystem::ApplyLightForTile(tile_t *tile, Color src, const Vector &worldpos, int x, int y)
{
    auto &tile_pos = tile->m_vecPosition;
    auto vox_pos = ivec3{
        std::clamp((worldpos.x - tile_pos.x) * 2.f, 0.f, 2.f), std::clamp((worldpos.y - tile_pos.y) * 2.f, 0.f, 2.f), std::clamp(worldpos.z, 0.f, 2.f)};

    // need to determine pos based on ray dir and mappos, and then z via  drawstart/end

    auto vox = tile->getVoxelAt(vox_pos.x, vox_pos.y, vox_pos.z);

    SetPixel(x, y, vox->m_light);
}

Color CLightingSystem::GetLightForTile(tile_t *tile)
{

    Vector tile_pos = {tile->m_vecPosition.x + 0.5f, tile->m_vecPosition.y + 0.5f, 0.25f};

    Color total_light = MaxDark();

    for (auto &entry : light_list)
    {
        Color to_set = Color::None();
        auto light = entry.second;

        auto light_pos = light->GetPosition();
        auto delta = light_pos - tile_pos;
        float range = light->GetRange();

        float distanceSquared = delta.LengthSqr();

        if (distanceSquared <= range * range)
        {
            float attenuation = 1.0f / (1.0f + params.a * sqrt(distanceSquared) + params.b * distanceSquared);
            attenuation = std::min(std::max(attenuation, params.minIntensity), 1.0f);

            Color lightColor = light->GetColor();
            float brightFactor = light->GetIntensity() * light->GetBrightness() * params.brightFactorMod;
            float alphaFactor = 1.0f - pow(std::min(brightFactor, 1.0f), 2) * params.alphaFactorMod;
            lightColor.a(static_cast<uint8_t>(lightColor.a() * attenuation * alphaFactor * params.finalAlphaMod));
            dbg(" lightA %i bf %.3f af %.3f, atn %.3f", lightColor.a(), brightFactor, alphaFactor, attenuation);
            switch (params.mergeMethod)
            {
            case 1:
                total_light = MergeLightColors(lightColor, total_light);
                break;
            case 0:
            default:
                total_light = MergeLightColors(total_light, lightColor);
            }
            total_light.a(lightColor.a());
        }
    }
    dbg(" final %i", total_light.a());

    return total_light;
}

void CLightingSystem::CalculateLighting()
{
    static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
    log("Building lighting info");
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    Timer_t gen_time(IEngineTime->GetCurTime());
    int num = 0;

    auto &level = ILevelSystem->m_Level;
    auto &world = level->world;
     for(auto& entry : light_list){
        entry.second->rays.clear();
    }
    for (auto &row : world)
    {
        for (auto &tile : row)
        {
            for (int x = 0; x < TILE_SECTORS; ++x)
                for (int y = 0; y < TILE_SECTORS; ++y)
                    for (int z = 0; z < TILE_SECTORS; ++z)
                        tile.sectors[x][y][z].m_light = MaxDark();
        }
    }

         
    for (auto &row : world)
    {
        for (auto &tile : row)
        {
            CalculateTileLightData(&tile);
            num++;
        }
    }
    gen_time.Update(IEngineTime->GetCurTime());
    auto tile_time = gen_time.Elapsed();
    log("made lightdata for %i tiles, %i voxels in %i ms", num, num * 9, tile_time.ms());
    for (auto &row : world)
    {
        for (auto &tile : row)
        {
            CalculateLerpLightData(&tile);
            num++;
        }
    }
    for(auto& entry : light_list){
        log("light %s cast %i rays", entry.first.c_str(), entry.second->rays.size());
    }
    gen_time.Update(IEngineTime->GetCurTime());
    log("made lerp light data for %i tiles, %i voxels & their neighbors in %i ms", num / 2, num * 9, gen_time.Elapsed().ms() - tile_time.ms());

    log("Built lighting info in %i ms for map %s with #%li lights", gen_time.Elapsed().ms(), ILevelSystem->m_Level->getName().c_str(), light_list.size());

    
}
void CLightingSystem::CalculateTileLightData(tile_t *tile)
{
   // if(tile->m_nType == Level::Tile_Wall) return;
    // pos is NW corner
    Vector pos = {tile->m_vecPosition.x, tile->m_vecPosition.y, 0.f}; // int vector2 to float vector3

    for (int x = 0; x < TILE_SECTORS; ++x)
        for (int y = 0; y < TILE_SECTORS; ++y)
            for (int z = 0; z < TILE_SECTORS; ++z)
            {
                Vector rel_pos = tile->getSectorCenterRelativeCoords(x, y, z);

                Vector world_pos = pos + rel_pos;
                tested_points.push_back(world_pos);
                Color vox_light = GetLightAtPoint(world_pos);
                if(tile->m_vecPosition.x == 13 && tile->m_vecPosition.y == 22)
                    log("%s %i %i %i", vox_light.s().c_str(), x, y, z);
                tile->sectors[x][y][z].m_light = vox_light;
            }
}

void CLightingSystem::CalculateLerpLightData(tile_t *tile)
{
   // if(tile->m_nType == Level::Tile_Wall) return;
    Vector pos = {tile->m_vecPosition.x, tile->m_vecPosition.y, 0.f};
    for (int x = 0; x < TILE_SECTORS; ++x)
        for (int y = 0; y < TILE_SECTORS; ++y)
            for (int z = 0; z < TILE_SECTORS; ++z)
            {
                auto voxel = tile->getVoxelAt(x, y, z);
                FindNeighborColors(tile, voxel, x, y, z);
                voxel->m_light = CombineWithNeighbors(voxel);
                
                if(tile->m_vecPosition.x == 13 && tile->m_vecPosition.y == 22)
                    log("LERP %s %i %i %i", voxel->m_light.s().c_str(), x, y, z);
            }
}

/*
1..6
n,e,s,w,u,d
*/
Color CLightingSystem::getNeighborColor(tile_t *tile, const ivec3 &rel, int dir)
{
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    if (rel.x < 0 || rel.x >= 3 || rel.y < 0 || rel.y >= 3 || rel.z < 0 || rel.z >= 3)
    {
        // in another tile
        auto nbr_tile = ILevelSystem->GetTileNeighbor(tile, dir);
        if (nbr_tile == nullptr)
            return Color::None();

        ivec3 offset = {3, 3, 3};
        ivec3 nbr_pos = (rel + offset) % 3;

        auto vox = nbr_tile->getVoxelAt(nbr_pos.x, nbr_pos.y, nbr_pos.z);
        if (vox == nullptr)
        {
            Error("our assert passed in getVoxelAt but we got a nullptr back, {%i, %i, %i}", nbr_pos.x, nbr_pos.y, nbr_pos.z);
            return Color::None();
        }
        return vox->m_light;
    }

    auto vox = tile->getVoxelAt(rel.x, rel.y, rel.z);

    if (vox)
    {
        return vox->m_light;
    };

    return Color::None();
}
void CLightingSystem::FindNeighborColors(tile_t *tile, voxel_t *voxel, int x, int y, int z)
{
    ivec3 coords = {x, y, z};
    std::vector<ivec3> relcoords = {
        // north, east, south, west (2d)
        {0, -1, 0},
        {1, 0, 0},
        {0, 1, 0},
        {-1, 0, 0},
    };
    // up +z down -z (NEVER IN ADJACENT TILES)
    ivec3 zcoords[] = {{0, 0, 1}, {0, 0, -1}};

    // not the prettiest way to do this
    ivec3 nbr_N = coords + relcoords[NORTH];
    Color north = getNeighborColor(tile, nbr_N, NORTH);

    ivec3 nbr_E = coords + relcoords[EAST];
    Color east = getNeighborColor(tile, nbr_E, EAST);

    ivec3 nbr_S = coords + relcoords[SOUTH];
    Color south = getNeighborColor(tile, nbr_S, SOUTH);

    ivec3 nbr_W = coords + relcoords[WEST];
    Color west = getNeighborColor(tile, nbr_W, WEST);

    Color up = Color::None();
    Color down = Color::None();
    if (z < 2)
    {
        auto vox = tile->getVoxelAt(x, y, z + 1);
        if (vox)
        {
            up = vox->m_light;
        };
    }
    if (z > 0)
    {
        auto vox = tile->getVoxelAt(x, y, z - 1);
        if (vox)
        {
            down = vox->m_light;
        };
    }

    std::vector<Color> neighbors = {
        north, east, south, west, up, down};
    voxel->m_neighborsize = 0;
    for (int i = 0; i < 6; ++i)
    {
        auto clr = neighbors.at(i);
        if (clr == Color::None())
            continue;

        voxel->m_neighbors[i] = clr;
        voxel->m_neighborsize++;
    }

    if (voxel->m_neighborsize < 3)
    {
        Error("voxel w/ only %i neighbors, tile{%i %i} voxel{%i %i %i} ", voxel->m_neighborsize, tile->m_vecPosition.x, tile->m_vecPosition.y, x, y, z);
    }
}

bool CLightingSystem::CastRayToPoint(CLight *light, const Vector &point, float maxDistance, float step)
{
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    auto light_pos = light->GetPosition();
    auto delta = light_pos - point;
    auto light_tile = ILevelSystem->GetTileAt(light_pos.x, light_pos.y);
    Vector2 rayDir = delta.Normalize();
    Vector2 ray = light_pos;

  //idk man just use DDA and it will work i bet

  /*
  WALLS CANT LOOK INSIDE THEMSELVES FOR LIGHTING INFO!!!!
  
  
  */
    while (Vector(ray - light_pos ).Length2D() <= maxDistance)
    {
        ray = ray + (rayDir * step);

       dbg("ray pos[%.1f %.1f] goal [%.1f %.1f] start [%.1f %.1f]", ray.x, ray.y, point.x, point.y, light_pos.x, light_pos.y);

        auto tile = ILevelSystem->GetTileAt(IVector2(ray.x, ray.y));
       // if (tile == light_tile)
       // {
            //light->rays.push_back( {ray, Vector2(point), (tile->m_nType == Level::Tile_Empty) });
            //return  (tile->m_nType == Level::Tile_Empty);
        //}
       
        if (tile != nullptr)
        {
             if(ILevelSystem->GetTileAt(IVector2(ray.x + step, ray.y))->m_nType != Level::Tile_Empty ){ //bad exec right idea
                if(ILevelSystem->GetTileAt(IVector2(ray.x , ray.y + step))->m_nType != Level::Tile_Empty ){
                     light->rays.push_back( {ray, Vector2(point), false });
                       // return false;
                }
             }
            if (tile->m_nType != Level::Tile_Empty)
            {
                light->rays.push_back( {ray, Vector2(point), false });
                return false;
               
            }
        }
       // if (ray.x < 0 || ray.y < 0 || ray.x > MAP_SIZE || ray.y > MAP_SIZE){
           // light->rays.push_back( {ray, Vector2(point), false });
          //  return false;
       // }
            
    }
    auto tile = ILevelSystem->GetTileAt(IVector2(ray.x, ray.y));
     if (tile->m_nType != Level::Tile_Empty)
            {
                light->rays.push_back( {ray, Vector2(point), false });
                return false;
               
            }
    light->rays.push_back( {ray, Vector2(point), true });
    tested_points.erase(std::remove(tested_points.begin(), tested_points.end(), point), tested_points.end());
    return true;
}

Color CLightingSystem::GetLightAtPoint(const Vector &point)
{
    /*
      Lighting Specs
      alpha represents darkness or shadow, then brighter light should mean a lower alpha value (more transparent).
      intensity can make this alpha change for brighter light, meaning more of the light color is applied

      intensity and brightness range from 0.0 (min) - (1.0) max

      the default no light value is MaxDark, defined below
      */
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    Color total_light = MaxDark();

    for (auto &entry : light_list)
    {
        Color to_set = Color::None();
        auto light = entry.second;

        auto light_pos = light->GetPosition();
        auto delta = light_pos - point;
        float range = light->GetRange();
        // CalculateLightInfluence(light, point); //eventually raycast
        float distance = delta.Length2D();

        if (distance <= range )
        {
           
            if (distance <= 0.01f || CastRayToPoint(light, point, distance, 0.5f))
            {
                 
                float attenuation = 1.0f / (1.0f + params.a * sqrt(distance) + params.b * distance); //No longer squared 
                attenuation = std::min(std::max(attenuation, params.minIntensity), 1.0f);

                Color lightColor = light->GetColor();
                float brightFactor = light->GetIntensity() * light->GetBrightness() * params.brightFactorMod;
                float alphaFactor = 1.0f - pow(std::min(brightFactor, 1.0f), 2) * params.alphaFactorMod;
                lightColor.a(static_cast<uint8_t>(lightColor.a() * attenuation * alphaFactor * params.finalAlphaMod));
                dbg(" lightA %i bf %.3f af %.3f, atn %.3f", lightColor.a(), brightFactor, alphaFactor, attenuation);
                switch (params.mergeMethod)
                {
                case 1:
                    total_light = MergeLightColors(lightColor, total_light);
                    break;
                case 0:
                default:
                    total_light = MergeLightColors(total_light, lightColor);
                }
                total_light.a(lightColor.a());
            }
        }
    }
    dbg(" final %i", total_light.a());

    return total_light;
}

Color CLightingSystem::CombineWithNeighbors(voxel_t *voxel)
{
    Color combinedColor = voxel->m_light;
    int count = 1; // Start with the voxel itself

    for (int i = 0; i < voxel->m_neighborsize; ++i)
    {
        if (voxel->m_neighbors[i] != Color::None())
        {
            combinedColor = LinearInterpolate(combinedColor, voxel->m_neighbors[i], params.interpFraction);
            count++;
        }
    }

    /*
        //combinedColor = combinedColor +  voxel->m_neighbors[i];

     // Average out the colors by dividing each component
        uint8_t avgR = ((combinedColor >> 24) & 0xFF) / count;
        uint8_t avgG = ((combinedColor >> 16) & 0xFF) / count;
        uint8_t avgB = ((combinedColor >> 8) & 0xFF) / count;
        uint8_t avgA = (combinedColor & 0xFF) / count;

        Color ret = Color(avgR, avgG, avgB, avgA);
        //log("%s", ret.s().c_str());
        return ret; */
    // Average out the colors
    return combinedColor / static_cast<float>(count);
}

Color CLightingSystem::LinearInterpolate(Color start, Color end, float fraction)
{

    uint8_t interpR = static_cast<uint8_t>(start.r() + fraction * (end.r() - start.r()));
    uint8_t interpG = static_cast<uint8_t>(start.g() + fraction * (end.g() - start.g()));
    uint8_t interpB = static_cast<uint8_t>(start.b() + fraction * (end.b() - start.b()));

    uint8_t interpA = static_cast<uint8_t>(start.a() + fraction * (end.a() - start.a()));
    return Color(interpR, interpG, interpB, interpA);
}
