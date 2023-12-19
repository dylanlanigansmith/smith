#pragma once
#include <entity/CBaseEntity.hpp>
#include <engine/engine.hpp>

#include <data/level.hpp>

#include ENTREG_INC

class CBaseDoorControl;

struct door_data
{
    enum door_state : int {
        DoorState_Invalid = 0, //not setup
        DoorState_Closed,
        DoorState_Open,
        DoorState_Opening,
        DoorState_Closing,
    };
    enum door_dir{ //closed->open change direction
            DoorDir_Default = 0,
            DoorDir_RightToLeft,
            DoorDir_LeftToRight, 
            DoorDir_Up, //down->up
            DoorDir_Down, //up->down
            DOORDIR_SIZE
        };
    int m_state;
    looptick_t m_startTime;
    struct {
        
        float m_speed; //% a tick
        int m_direction;
        bool m_startClosed;
        void defaults(){
            m_speed = 1.f;
            m_direction = 0;
            m_startClosed = true;
        }
    }params;
    float m_ajar; //0 closed 100 open
    tile_t* m_tile;
    
    door_data() : m_state(DoorState_Invalid), m_startTime(0), m_ajar(0.f) {}
    ~door_data(){
        if(m_tile != nullptr){
            if(m_tile->m_pState != nullptr){
                delete m_tile->m_pState; //probably not a good idea 
            }
        }
    }
    void setup(CBaseDoorControl* m_parent, const IVector2& tile_pos)  {

        static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
        assert(ILevelSystem->ValidTilePosition(tile_pos));
        m_tile = ILevelSystem->GetTileAtFast(tile_pos);
        if(m_tile == nullptr){
            gError("somehow we have a valid tile position but a null tile at %d %d", tile_pos.x, tile_pos.y); return;
        }
        params.defaults();
        m_state = (params.m_startClosed) ? DoorState_Closed : DoorState_Open;
        m_ajar =  (params.m_startClosed) ? 0.f : 100.f;
        m_tile->m_pState = new tile_state(m_parent);
        m_tile->m_pState->m_doorctl = m_parent;
    } 

    inline bool inTransition() const{
        return (m_state == DoorState_Closing || m_state == DoorState_Opening);
    }
    inline void Open(){
        if(m_state != DoorState_Open)
            m_state = DoorState_Opening;
    }
    inline void Close(){
        if(m_state != DoorState_Closed)
            m_state = DoorState_Closing;
    }
};

class CBaseDoorControl : public CBaseEntity, private CLogger
{
    friend class CEditor;
public:
   
    CBaseDoorControl(int m_iID) : CBaseEntity(m_iID), CLogger(this, std::to_string((m_iID))) {} 
    virtual ~CBaseDoorControl(){}
 
    
    virtual void OnUpdate();
  
    virtual void OnCreate(){
        ENT_SETUP();
        m_door.m_state = m_door.DoorState_Opening;

    }
    virtual void OnDestroy(){

    }
    virtual float GetBounds() const { return 0.34f; }
    virtual void OnCollisionWith(CBaseEntity* hit) {}
    virtual void WhenCollidedBy(CBaseEntity* hitter) {}
    virtual bool IsBlocking() const { return false; }
    virtual bool IsRenderable() { return false; }

    inline float GetDrawState() { return m_door.m_ajar; }

    inline auto GetDoorState() const { return m_door.m_state; }
    inline auto& GetDoor() { return m_door; } //for editor

    virtual void SetTarget(const IVector2& tilepos){
        SetupSlave(tilepos);
    }
    auto Direction() const { return m_door.params.m_direction; }
    auto IsSetup() const { return ( m_door.m_state != m_door.DoorState_Invalid && m_door.m_tile != nullptr); }
    auto UsingYAxis() const { return m_usingY; }
    auto IsOpen() const { return m_door.m_state ==  m_door.DoorState_Open; }


    virtual bool IsShootable() const { return true; }
    virtual void OnHit(int damage, int position = 0) {
       if(!IsOpen()) m_door.Open();
    }

    virtual void ToggleState(){
        if(IsOpen()) m_door.Close();
        else m_door.Open();
    }
protected:
    virtual void OnSetPosition(const Vector2& old_pos, const Vector2& new_pos) {}

    virtual void SetupSlave(const IVector2& tilepos){
        m_tilepos = tilepos;
        m_door.setup(this, tilepos);
        if(!IsSetup()){
            warn("setup failed"); return;
        } 
        assert(m_door.m_tile->m_pState->m_doorctl == this);
        dbg("now controlling tile {%d %d}", m_door.m_tile->m_vecPosition.x, m_door.m_tile->m_vecPosition.y);
    }
protected:
    IVector2 m_tilepos;
    door_data m_door;
    bool m_usingY;
private:
    REGISTER_DEC_ENT(CBaseDoorControl);
};