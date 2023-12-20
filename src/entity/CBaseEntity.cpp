#include "CBaseEntity.hpp"
#include <engine/engine.hpp>

void CBaseEntity::SetPosition(double x, double y, double z)
{

    IVector2 newmapPos = {int(x), int(y)};

    Vector2 old_pos = GetPosition();
    auto new_tile = ILevelSystem->GetTileAt(newmapPos);

    IVector2 mapPos = {int(m_vecPosition.x), int(m_vecPosition.y)};
    auto cur_tile = ILevelSystem->GetTileAt(mapPos);

    if(cur_tile != nullptr && cur_tile->m_occupants.size() != 0 ){
        auto& occs = cur_tile->m_occupants;
        occs.erase(std::remove(occs.begin(), occs.end(), m_iID), occs.end());
    }
    new_tile->m_occupants.push_back(m_iID);
    m_vecPosition = { x,y,z}; 

    OnSetPosition(old_pos, m_vecPosition );

};