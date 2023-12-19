#include "CBaseDoorControl.hpp"

REGISTER_DEF_ENT(CBaseDoorControl);

void CBaseDoorControl::OnUpdate()
{
        if(!IsSetup() || !m_door.m_tile->IsThinWall() || !m_door.m_tile->HasState() )  { 
            warn("door to control not thin wall or lacks state! did you call CBaseDoorControl::SetTarget() ?"); return;
        }
        
        static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
        if(m_door.inTransition())
        {
            float change = ((m_door.m_state == m_door.DoorState_Opening) ? (1.f) : (-1.f));
            m_door.m_ajar += change * m_door.params.m_speed;

            if(m_door.m_ajar <= 0.f){
                m_door.m_state = m_door.DoorState_Closed;
                m_door.m_ajar = 0.f;
                m_door.m_tile->m_flDoor = m_door.m_ajar / 100.f;
                ILightingSystem->RegenerateLightingForDynamicTile(m_door.m_tile);
            }
            if(m_door.m_ajar >= 100.f){
                m_door.m_state = m_door.DoorState_Open;
                m_door.m_ajar = 100.f;
                m_door.m_tile->m_flDoor = m_door.m_ajar / 100.f;
                ILightingSystem->RegenerateLightingForDynamicTile(m_door.m_tile);
            }
        }
        else{
            switch( m_door.m_state)
            {
                case door_data::DoorState_Open:
                    m_door.m_tile->SetNoClip(true); break;
                case door_data::DoorState_Closed:
                default:
                    m_door.m_tile->SetNoClip(false);
            }
        }
        m_usingY = (m_door.m_state != door_data::DoorState_Closed && 
                    (m_door.params.m_direction == door_data::DoorDir_Up || m_door.params.m_direction == door_data::DoorDir_Down));
 
        m_door.m_tile->m_flDoor = m_door.m_ajar / 100.f;
}