#include "ILightingSystem.hpp"
#include <interfaces/ILevelSystem/ILevelSystem.hpp>
#include <engine/engine.hpp>
#include "gen/LightData.hpp"

CLevelSystem* CLightingSystem::LevelSystem = nullptr;
light_reg_t *CLightingSystem::light_class = nullptr;
CLightingSystem *CLightingSystem::_interface()
{
    static auto LightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
    return LightingSystem;
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

void CLightingSystem::RegenerateLightingForDynamicTile(tile_t *tile)
{

    static auto RegenProfiler = IEngineTime->AddProfiler("CLightingSystem::RegenerateLightingForDynamicTile()"); 
    RegenProfiler->Start();
    if(!tile || !tile->HasState()) return;
    if(tile->m_pState->light_pts.empty()) return;
    auto& light_pts = tile->m_pState->light_pts;

    for(const auto& update : light_pts){
        auto& pt = update.first;
        LightData::UpdateLightForPoint(pt.x, pt.y, pt.z, update.second); //this should fix static map too idk
    }
    log("updated size %li", light_pts.size());
    RegenProfiler->EndAndLog();
}
void CLightingSystem::RegenerateLightingForDynamicLight(CLight* light)
{

    static auto RegenProfiler = IEngineTime->AddProfiler("CLightingSystem::RegenerateLightingForDynamicLight()"); 
    RegenProfiler->Start();
    if(!light) return;
    if(!light->InfluenceSize()) return;
   

    for(const auto& pt : light->GetInfluence()){
        LightData::UpdateLightForPoint(pt.x, pt.y, pt.z, light);
         auto idx = LightData::WorldToMapIndex(pt);
        staticmap[idx.x][idx.y][idx.z] = lightmap[idx.x][idx.y][idx.z];
    }
   // RegenProfiler->EndAndLog();
     RegenProfiler->End();
   
}

void CLightingSystem::RegenerateLightingForTempLight(CLight* light, bool restore)
{

    static auto RegenProfiler = IEngineTime->AddProfiler("CLightingSystem::RegenerateLightingForTempLight()"); 
    RegenProfiler->Start();
    if(!light) return;
    if(!light->InfluenceSize()) return;
   

    for(const auto& pt : light->GetInfluence()){
        if(restore)
        {
            //LightData::UpdateLightForPoint(pt.x, pt.y, pt.z);

            auto idx = LightData::WorldToMapIndex(pt);
            lightmap[idx.x][idx.y][idx.z] = staticmap[idx.x][idx.y][idx.z];
        }
        else
            LightData::UpdateLightForPoint(pt.x, pt.y, pt.z, light);
    }
    RegenProfiler->End();
   // RegenProfiler->EndAndLog();
}

void CLightingSystem::RegenerateLighting()
{

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

    log("Building lighting info");

    LevelSystem = ILevelSystem;
    static auto LightGenProfiler = IEngineTime->AddProfiler("CLightingSystem::CalculateLighting()");
    LightGenProfiler->HideFromEditor();
    LightGenProfiler->Start();
    SetLogFileOnly(true);

    LightData::Calculate2();
    auto &level = ILevelSystem->m_Level;
    auto &world = level->world;
    for (auto &row : world)
    {
        for (auto &tile : row)
        {
            // CalculateLerpLightData(&tile, false);
        }
    }
    SetLogFileOnly(false);
    
    staticmap = lightmap;
    //std::copy(&lightmap[0][0][0], &lightmap[0][0][0] + MAP_SIZE * 10 * MAP_SIZE * 10 * 10, &staticmap[0][0][0]);

    LightGenProfiler->EndAndLog();
    return;
}

/*
n,e,s,w,u,d
*/
Color CLightingSystem::getNeighborColor(tile_t *tile, const ivec3 &rel, int dir)
{

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

nlohmann::json CLightingSystem::ToJSON()
{
    auto lights = json::object();

    for (auto &entry : light_list)
    {

        auto light = entry.second;
        if(light->IsTemporary()) continue;
        auto pos = light->GetPosition();
        auto light_json = json::array({light->GetName(), (uint32_t)light->GetColor(), light->GetBrightness(), light->GetIntensity(), light->GetRange(), json::array({pos.x, pos.y, pos.z}), (uint64_t)0u, 1.f, 1.f});
        lights.emplace(entry.first, light_json);
    }

    return lights;
}

void CLightingSystem::FromJSON(const nlohmann::json &j)
{
    for (auto &ltj : j)
    {
        auto light = AddLightByClassname(ltj.at(0));
        // we can just crash if this nullptrs

        light->SetColor(Color(uint32_t(ltj.at(1))));
        light->SetBrightness((float)ltj.at(2));
        light->SetIntensity((float)ltj.at(3));
        light->SetRange((float)ltj.at(4));

        auto j_pos = nlohmann::json::array();
        j_pos = ltj.at(5);
        light->SetPosition({
            (float)j_pos.at(0),
            (float)j_pos.at(1),
            (float)j_pos.at(2),
        });

        dbg("Added light from file: %s, %s, pos{%.1f, %.1f, %.1f}, brt %.3f, int %.3f, rng%.3f",
            light->GetName().c_str(), light->GetColor().s().c_str(), light->GetPosition().x, light->GetPosition().y, light->GetPosition().z, light->GetBrightness(), light->GetIntensity(), light->GetRange());
    }

    status("added %li lights from json", light_list.size());
}

void CLightingSystem::OnPreLevelChange()
{
    for(auto itr = light_list.begin(); itr != light_list.end(); )
    {
        if(itr->second != nullptr && !itr->second->IsTemporary()) // Check condition
        {
            delete itr->second; // Delete the CLight object
            itr = light_list.erase(itr); // Erase element and get next iterator
        }
        else
        {
            ++itr; // Move to next element
        }
    }

   // light_list.clear(); //this works fine 
   // light_list.erase(std::remove_if(light_list.begin(), light_list.end(), [](const std::pair<std::string, CLight*>& itr){  return !itr.second->IsTemporary(); }), light_list.end());
    //^^ error: use of deleted function ‘std::pair<_T1, _T2>& std::pair<_T1, _T2>::operator=(const std::pair<_T1, _T2>&) [with _T1 = const std::__cxx11::basic_string<char>; _T2 = CLight*]’
    log("cleared light list, %li temporary objects kept", light_list.size());
}
