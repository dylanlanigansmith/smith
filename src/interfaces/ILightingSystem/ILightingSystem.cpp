#include "ILightingSystem.hpp"
#include <interfaces/ILevelSystem/ILevelSystem.hpp>
#include <engine/engine.hpp>
#include "gen/LightData.hpp"
light_reg_t* CLightingSystem::light_class = nullptr;
CLightingSystem *CLightingSystem::_interface()
{
    static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
    return ILightingSystem;
}
Color CLightingSystem::CalculateLightInfluence(CLight *light, const Vector &point)
{

    return Color::None();
}

void CLightingSystem::OnEngineInitFinish()
{
    Debug(false);
    StartLogFileForInstance("lighting.log", false);
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



void CLightingSystem::CalculateLighting()
{
    static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
    log("Building lighting info");
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");

    static auto LightGenProfiler = IEngineTime->AddProfiler("CLightingSystem::CalculateLighting()");
    LightGenProfiler->Start();
    SetLogFileOnly(true);
    LightData ld(this);
    ld.Calculate();
    auto &level = ILevelSystem->m_Level;
    auto &world = level->world;
    for (auto &row : world)
        {
            for (auto &tile : row)
            {
                //CalculateLerpLightData(&tile, false);
               
            }
        }
    SetLogFileOnly(false);
    LightGenProfiler->EndAndLog();

    return;

/*
    Timer_t gen_time(IEngineTime->GetCurTime());
    int num = 0;
    SetLogFileOnly(true);
    auto &level = ILevelSystem->m_Level;
    auto &world = level->world;
    for (auto &entry : light_list)
    {
        entry.second->rays.clear();
    }
    for (auto &row : world)
    {
        for (auto &tile : row)
        {
            for (int x = 0; x < TILE_SECTORS; ++x)
                for (int y = 0; y < TILE_SECTORS; ++y)
                    for (int z = 0; z < TILE_SECTORS; ++z)
                        tile.sectors[x][y][z].m_light = Color::None();
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
    // just add a pass that fixes all the wall tiles??

    gen_time.Update(IEngineTime->GetCurTime());
    auto tile_time = gen_time.Elapsed();
    log("made lightdata for %i tiles, %i voxels in %i ms", num, num * 9, tile_time.ms());


    int numLerps = 1;

    while(numLerps > 0)
    {    
        for (auto &row : world)
        {
            for (auto &tile : row)
            {
                CalculateLerpLightData(&tile, false);
                num++;
            }
        }
       
        log("lerp round %i / %i", numLerps, num);
        numLerps--;
    } 

    SetLogFileOnly(false);
    for (auto &entry : light_list)
    {
        log("light %s cast %li rays", entry.first.c_str(), entry.second->rays.size());
    }
    gen_time.Update(IEngineTime->GetCurTime());
    log("made face/lerp light data for %i tiles, %i voxels & their neighbors in %i ms", num / 2, num * 9, gen_time.Elapsed().ms() - tile_time.ms());

    log("Built lighting info in %i ms for map %s with #%li lights", gen_time.Elapsed().ms(), ILevelSystem->m_Level->getName().c_str(), light_list.size());

    SetLogFileOnly(true);
*/
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

                Vector world_pos =  pos + rel_pos;
                assert(floor(world_pos.x) == (double)tile->m_vecPosition.x);
                tested_points.push_back(world_pos);
                Color vox_light = GetLightAtPoint(world_pos);
                //if (tile->m_vecPosition.x == 13 && tile->m_vecPosition.y == 22)
                //    log("%s %i %i %i", vox_light.s().c_str(), x, y, z);
                tile->sectors[x][y][z].m_light = vox_light;
            }
}

void CLightingSystem::CalculateLerpLightData(tile_t *tile, bool set)
{
    // if(tile->m_nType == Level::Tile_Wall) return;
    Vector pos = {tile->m_vecPosition.x, tile->m_vecPosition.y, 0.f};
    for (int x = 0; x < TILE_SECTORS; ++x)
        for (int y = 0; y < TILE_SECTORS; ++y)
            for (int z = 0; z < TILE_SECTORS; ++z)
            {
                auto voxel = tile->getVoxelAt(x, y, z);
                FindNeighborColors(tile, voxel, x, y, z);
                if(!set)
                    CombineWithNeighbors(voxel);

                // if(tile->m_vecPosition.x == 13 && tile->m_vecPosition.y == 22)
                // log("LERP %s %i %i %i", voxel->m_light.s().c_str(), x, y, z);
            }
if(set)
    CombineWithNeighbors(tile);
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
        if (tile->m_nType != Level::Tile_Empty && nbr_tile->m_nType != Level::Tile_Empty)
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
    auto delta = point - light_pos;
    auto light_tile = ILevelSystem->GetTileAt(IVector2::Rounded(light_pos.x, light_pos.y));
    Vector2 rayDir = delta.Normalize();
    Vector2 ray = light_pos;
    auto pttile = ILevelSystem->GetTileAt(point.x, point.y);
    if (pttile == nullptr)
        return false;
    double distToLight = Vector2(ray - light_pos).Length();

    double distToPoint = (point - ray).Length2D();
    int walls = 0;
    while ((ray - light_pos).Length() <= maxDistance)
    {
        
        ray = ray + (rayDir * step);
        distToLight = Vector2(ray - light_pos).Length();
        distToPoint = (point - ray).Length2D();
       
        dbg("ray pos[%.1f %.1f] goal [%.1f %.1f] start [%.1f %.1f]", ray.x, ray.y, point.x, point.y, light_pos.x, light_pos.y);

        auto tile = ILevelSystem->GetTileAt(IVector2(ray.x, ray.y));
        if (tile != nullptr)
        {
             auto tile2 = ILevelSystem->GetTileAt(IVector2(ray - (rayDir * 1.f/3.f )));
            if (tile->m_nType != Level::Tile_Empty && !tile->m_pTexture->isTransparent())
            { 
                walls++;
                if(tile2->m_nType == Level::Tile_Empty || tile2->m_pTexture->isTransparent() ){
                    if(distToPoint < 1.f){
                         tested_points.erase(std::remove(tested_points.begin(), tested_points.end(), point), tested_points.end());
                        light->rays.push_back({ray - (rayDir * 2 * step), Vector2(point), true}); return true;
                    }
                    
                }

               //light->rays.push_back({ray, Vector2(point), false});
                //return false;
            }
        }      
         if(distToPoint < step)
            break;
    }
    auto tile = ILevelSystem->GetTileAt(IVector2::Rounded(ray - (rayDir * step)));
    if (tile->m_nType != Level::Tile_Empty)
    {
        light->rays.push_back({ray - (rayDir * 2 * step), Vector2(point), false});

        return false;
    }
    light->rays.push_back({ray, Vector2(point), true});
   
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
        Vector2 delta = point - light_pos;
        float range = light->GetRange();
        // CalculateLightInfluence(light, point); //eventually raycast
        float distance = delta.Length();

        if (distance <= range)
        {

            if ( CastRayToPoint(light, point, distance, 0.1f))
            {
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
Color CLightingSystem::CombineWithNeighbors(tile_t* tile)
{
    
    for (int x = 0; x < TILE_SECTORS; ++x)
    {
        for (int y = 0; y < TILE_SECTORS; ++y)
        {
            for (int z = 0; z < TILE_SECTORS; ++z)
            {
                auto voxel = tile->getVoxelAt(x,y,z);
                if(!voxel || !tile) return 0;
                /*
                if(!tile->isEmpty() && false){
                    if(x == 1 && y == 1){
                        voxel->m_light = Color::None();
                         continue; //no tcenters
                    }
                    int face_ew = -1, face_ns = -1;
                    if(x == 0) face_ew = WEST;
                    else if(x== 2) face_ew = EAST;

                    if(y == 0) face_ns = NORTH;
                    else if(y == 2) face_ns = SOUTH;

                    //ensure that no bad influences are taking part.. this should rly be done earlier in processing

                    // north, east, south, west, up = 4, down = 5
                    if(z == 1){
                            voxel->m_neighbors[4] = Color::None(); //up
                            voxel->m_neighbors[5] = Color::None(); //down
                    }
                    if(face_ew == WEST){
                        voxel->m_neighbors[EAST] = Color::None();
                        if(y == 1){
                             voxel->m_neighbors[NORTH] = Color::None();
                             voxel->m_neighbors[SOUTH] = Color::None();
                        }
                    }
                    if(face_ew == EAST){
                        voxel->m_neighbors[WEST] = Color::None();
                        if(y == 1){
                             voxel->m_neighbors[NORTH] = Color::None();
                             voxel->m_neighbors[SOUTH] = Color::None();
                        }
                    }
                    if(face_ns == NORTH){
                        voxel->m_neighbors[SOUTH] = Color::None();
                        if(x == 1){
                             voxel->m_neighbors[WEST] = Color::None();
                             voxel->m_neighbors[EAST] = Color::None();
                        }
                    }
                    if(face_ns == SOUTH){
                        voxel->m_neighbors[NORTH] = Color::None();
                        if(x == 1){
                             voxel->m_neighbors[WEST] = Color::None();
                             voxel->m_neighbors[EAST] = Color::None();
                        }
                    }
                }
               */
                Color combinedColor = voxel->m_light;
                int count = 1; // Start with the voxel itself

                for (int i = 0; i < voxel->m_neighborsize; ++i)
                {
                    if (voxel->m_neighbors[i] != Color::None())
                    {
                        combinedColor = LinearInterpolate(combinedColor, voxel->m_neighbors[i], params.interpFraction);
                        if(true){
                            voxel->m_neighbors[i] = LinearInterpolate(combinedColor, voxel->m_light, 1.0 - params.interpFraction );
                            voxel->m_neighbors[i] = combinedColor / static_cast<float>(count);
                        }                       
                        count++;
                    }
                }
                voxel->m_light = combinedColor / static_cast<float>(count);
                
             }
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
    //return combinedColor 

    return Color::None();
}




static int welps = 0;
inline void WallHelper(tile_t* tile,  int dir)
{
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");

    auto nbr = ILevelSystem->GetTileNeighbor(tile, dir); //hopefully this function works
  
    if(nbr == nullptr) return;

   
    if(!nbr->isEmpty()) return;
    if(dir == WEST || dir == EAST)
    {
        for (int y = 0; y < TILE_SECTORS; ++y)
            for (int z = 0; z < TILE_SECTORS; ++z){
                welps++;
                switch(dir)
                {
                    case WEST:
                         //WEST FACE                             //EAST FACE of neighbr
                    tile->getVoxelAt(0,y,z)->m_light = nbr->getVoxelAt(2,y,z)->m_light; break;

                    case EAST:
                     tile->getVoxelAt(2,y,z)->m_light = nbr->getVoxelAt(0,y,z)->m_light; break;

                }
            }

        return;
    }
    if(dir == NORTH || dir == SOUTH)
    {
        for (int x = 0; x < TILE_SECTORS; ++x)
            for (int z = 0; z < TILE_SECTORS; ++z){
                switch(dir)
                {
                    welps++;
                    case NORTH:
                               //nface                   ==               sface of nbr
                    tile->getVoxelAt(x,0,z)->m_light = nbr->getVoxelAt(x,2,z)->m_light; break;

                    case SOUTH:
                     tile->getVoxelAt(x,2,z)->m_light = nbr->getVoxelAt(x,0,z)->m_light; break;

                }
            }

        return;
    }


}

void CLightingSystem::CalculateWallFaces(Color dark)
{
    
    welps = 0;
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    auto &level = ILevelSystem->m_Level;
    auto &world = level->world;
    if(dark != Color::None())
    {
        for (auto &row : world)
        {
            for (auto &tile : row)
            {
                if(tile.isEmpty()) continue; 
            
                for (int x = 0; x < TILE_SECTORS; ++x)
                    for (int y = 0; y < TILE_SECTORS; ++y)
                        for (int z = 0; z < TILE_SECTORS; ++z)
                            tile.sectors[x][y][z].m_light = dark;  //Reset all walls to dark
            }
        }
    }
    
    //north face: y = 0
    //south face y = 2
    //west face x = 0
    //east face x = 2
    //top/bottom shouldnt matter 

    for (auto &row : world)  for (auto &tile : row)
    {
        if(tile.isEmpty()) continue; //loop thru all tiles, skip air tiles
        for(int dir = NORTH; dir <= WEST; ++dir)
            WallHelper(&tile, dir);
    }
    
    status("did %i welps", welps);

}
inline std::vector<Color> WallFixHelper(tile_t* tile,  int dir)
{
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");

    auto nbr = ILevelSystem->GetTileNeighbor(tile, dir); //hopefully this function works
    
    std::vector<Color> ret;
    if(nbr == nullptr) return ret;

    
   
    if(!nbr->isEmpty()) return ret;

    std::vector<Color> face_colors;

    if(dir == WEST || dir == EAST)
    {
        for (int y = 0; y < TILE_SECTORS; ++y)
            for (int z = 0; z < TILE_SECTORS; ++z){
                
                switch(dir)
                {
                    case WEST:
                    face_colors.push_back(tile->getVoxelAt(0,y,z)->m_light );
                    break;

                    case EAST:
                     face_colors.push_back(tile->getVoxelAt(2,y,z)->m_light); break;

                }
            }

       return face_colors;
    }
    if(dir == NORTH || dir == SOUTH)
    {
        for (int x = 0; x < TILE_SECTORS; ++x)
            for (int z = 0; z < TILE_SECTORS; ++z){
                switch(dir)
                {
                   
                    case NORTH:
                               //nface                   ==               sface of nbr
                    face_colors.push_back(tile->getVoxelAt(x,0,z)->m_light ); break;

                    case SOUTH:
                     face_colors.push_back(tile->getVoxelAt(x,2,z)->m_light ); break;

                }
            }

        return face_colors;
    }

    return ret;
}
inline void WallFixSetter(tile_t* tile,  int dir, Color clr)
{
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");

   
  
   
    if(dir == WEST || dir == EAST)
    {
        for (int y = 0; y < TILE_SECTORS; ++y)
            for (int z = 0; z < TILE_SECTORS; ++z){
                welps++;
                switch(dir)
                {
                    case WEST:
                         //WEST FACE                             //EAST FACE of neighbr
                    tile->getVoxelAt(0,y,z)->m_light = clr; break;

                    case EAST:
                     tile->getVoxelAt(2,y,z)->m_light = clr; break;

                }
            }

        return;
    }
    if(dir == NORTH || dir == SOUTH)
    {
        for (int x = 0; x < TILE_SECTORS; ++x)
            for (int z = 0; z < TILE_SECTORS; ++z){
                switch(dir)
                {
                    welps++;
                    case NORTH:
                               //nface                   ==               sface of nbr
                    tile->getVoxelAt(x,0,z)->m_light = clr; break;

                    case SOUTH:
                     tile->getVoxelAt(x,2,z)->m_light = clr; break;

                }
            }

        return;
    }


}

void CLightingSystem::FixWallFaces(int method)
{
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    auto &level = ILevelSystem->m_Level;
    auto &world = level->world;

    for (auto &row : world)  for (auto &tile : row)
    {
        if(tile.isEmpty()) continue; //loop thru all tiles, skip air tiles
        for(int dir = NORTH; dir <= WEST; ++dir)
        {
            auto face_colors =  WallFixHelper(&tile, dir);
            if(face_colors.empty()) continue;
            if(method == 0)
            {
                 uint32_t r,g,b,a;
                uint8_t highest_alpha = 0;
                uint8_t lowest_alpha = 255;
                int count = 0;
                for(auto clr : face_colors)
                {
                    r += clr.r();
                    g += clr.g();
                    b += clr.b();
                    a += clr.a();
                    if(clr.a() > highest_alpha){
                        highest_alpha = clr.a();
                    }
                    if(clr.a() < lowest_alpha){
                        lowest_alpha = clr.a();
                    }
                    count++;

                }
                r /= count;
                g /= count;
                b /= count;
                a /= count;
                a = highest_alpha;
                Color avg = Color(r,g,b,a);
                WallFixSetter(&tile, dir, avg);
            }
           if(method == 1)
           {
                 uint32_t r,g,b,a;
                uint8_t highest_alpha = 0;
                uint8_t lowest_alpha = 255;
                int count = 0;
                for(auto clr : face_colors)
                {
                    r += clr.r();
                    g += clr.g();
                    b += clr.b();
                    a += clr.a();
                    if(clr.a() > highest_alpha){
                        highest_alpha = clr.a();
                    }
                    if(clr.a() < lowest_alpha){
                        lowest_alpha = clr.a();
                    }
                    count++;

                }
                r /= count;
                g /= count;
                b /= count;
                a /= count;
                a = (highest_alpha + lowest_alpha) / 2.f;
                Color avg = Color(r,g,b,a);
                 Color combinedColor = avg;
                int lcount = 1; // Start with the voxel itself
                for(auto clr : face_colors)
                {
                    combinedColor = LinearInterpolate(combinedColor, clr, params.interpFraction);
                    lcount ++;
                }
                combinedColor = combinedColor / static_cast<float>(lcount);
                
                 WallFixSetter(&tile, dir, combinedColor);
               
           }
        }
           
    }
}


nlohmann::json CLightingSystem::ToJSON()
{
    auto lights =  json::object();
    

    for(auto& entry : light_list){
        auto light = entry.second;
        auto pos = light->GetPosition();
        auto light_json = json::array({light->GetName(),  (uint32_t)light->GetColor(), light->GetBrightness(), light->GetIntensity(), light->GetRange(), json::array({pos.x, pos.y, pos.z}), (uint64_t)0u, 1.f, 1.f });
        lights.emplace(entry.first, light_json);
    }
    
    return lights;
}

void CLightingSystem::FromJSON(const nlohmann::json& j)
{
   for(auto& ltj : j)
   {
        auto light = AddLightByClassname(ltj.at(0));
        //we can just crash if this nullptrs

        light->SetColor(Color(uint32_t(ltj.at(1))));
        light->SetBrightness((float)ltj.at(2)  );
        light->SetIntensity((float)ltj.at(3)  );
        light->SetRange((float)ltj.at(4)  );

        auto j_pos = nlohmann::json::array();
        j_pos = ltj.at(5);
        light->SetPosition({(float)j_pos.at(0), (float)j_pos.at(1), (float)j_pos.at(2),});

        log("Added light from file: %s, %s, pos{%.1f, %.1f, %.1f}, brt %.3f, int %.3f, rng%.3f",
               light->GetName().c_str(), light->GetColor().s().c_str(),light->GetPosition().x, light->GetPosition().y, light->GetPosition().z, light->GetBrightness(), light->GetIntensity(), light->GetRange());

   }

   status("added %li lights from json", light_list.size());
}
