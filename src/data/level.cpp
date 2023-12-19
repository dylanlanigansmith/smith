#include "level.hpp"
#include <engine/engine.hpp>
#include <light/CLight.hpp>
#include <entity/level/CBaseDoorControl.hpp>


json CLevel::ToJSON()
{
    auto meta = json::array(); // player flags like starting weapons or somethin
    meta = {m_szLevelName, m_vecBounds.x, m_vecBounds.y, m_flCeilingHeight, m_flFloorHeight, m_vecPlayerStart.x, m_vecPlayerStart.y, (uint64_t)0x1337};

    auto j = json::object();
    j.emplace("metadata", meta);

    auto lights = json::object();
    static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
    auto lightdata = ILightingSystem->ToJSON();
    j.emplace("lighting", lightdata);

    auto tile_data = json::object();
    for (const auto &row : world)
    {
        for (const auto &tile : row)
        {
            auto t = TileToJson(tile);
            tile_data.emplace(std::string(tile.m_vecPosition.str()) + std::to_string(tile.m_nType), t);
        }
    }
    j.emplace("tile", tile_data);
    return j;
}

bool CLevel::FromJSON(const json &j) // entire file
{
    auto meta = j.at("metadata");
    m_szLevelName = meta.at(0);
    m_vecBounds.x = meta.at(1);
    m_vecBounds.y = meta.at(2);
    m_flCeilingHeight = meta.at(3);
    m_flFloorHeight = meta.at(4);
    m_vecPlayerStart.x = meta.at(5);
    m_vecPlayerStart.y = meta.at(6);
    m_nPlayerInfo = meta.at(7);
    MakeEmptyLevel(HTEXTURE_INVALID); // now that we have bounds
    auto tile_data = j.at("tile");
    int amt = 0;
    for (const auto &key : tile_data)
    {
        auto tile = JsonToTile(key);
        AddTile(tile);
        amt++;
    }
     static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
    auto light_data = j.at("lighting");
    ILightingSystem->FromJSON(light_data);
    return (amt > 0);
}