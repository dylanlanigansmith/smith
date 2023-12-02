#include "CBaseEntity.hpp"
#include <engine/engine.hpp>

void CBaseEntity::SetPosition(double x, double y, double z)
{
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    IVector2 newmapPos = {int(x), int(y)};


    auto new_tile = ILevelSystem->GetTileAt(newmapPos);

    IVector2 mapPos = {int(m_vecPosition.x), int(m_vecPosition.y)};
    auto cur_tile = ILevelSystem->GetTileAt(mapPos);

    if(cur_tile != nullptr && cur_tile->m_occupants.size() != 0 ){
        auto& occs = cur_tile->m_occupants;
        occs.erase(std::remove(occs.begin(), occs.end(), m_iID), occs.end());
    }
    new_tile->m_occupants.push_back(m_iID);
    m_vecPosition = { x,y,z}; 

};