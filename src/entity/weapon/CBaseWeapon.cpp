#include "CBaseWeapon.hpp"
#include <engine/engine.hpp>
void CBaseWeapon::OnSetOwnerEntity()
{
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
    m_pOwner = IEntitySystem->GetEntity<CBaseRenderable>(m_iOwnerID);
} // todo:s