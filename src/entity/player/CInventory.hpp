#pragma once
#include <entity/weapon/CBaseWeapon.hpp>
#include <logger/logger.hpp>



template < size_t INV_SLOTS >
class CInventory : public CLogger
{
public:
    CInventory() : CLogger(this), m_activeSlot(0), m_filledSlots(0)  {
        for(auto& wep : m_contents){ //for posterity's sake
            wep = nullptr;
        }
    }
    virtual ~CInventory() {
        for(auto& wep : m_contents){
            if(wep != nullptr)
                delete wep;//i dont love that player creates gun, but inv destroys it but.. 
        }
    }

    auto& At(uint8_t slot) { return m_contents[slot]; } 
    auto& Active() { return m_contents[m_activeSlot]; }
    bool Switch(uint8_t slot) { if(SlotFilled(slot)) m_activeSlot = slot; return (SlotFilled(slot)); }

    void AddItem(CBaseWeapon* to_add) { dbg("added %s at %d", to_add->GetName().c_str(), m_filledSlots); m_contents[ m_filledSlots] = to_add; m_activeSlot = m_filledSlots; m_filledSlots++; } 
    void RemoveItem(uint8_t slot) {}
    auto MaxSize() const { return m_contents.size(); }
    auto Size() const { return m_filledSlots; }
    bool HasSpace() const { return m_filledSlots < MaxSize() - 1; } //bug?
    bool SlotFilled(uint8_t slot) { return (m_contents[slot] != nullptr);}
    auto& contents() { return m_contents; }

    void AddAmmo(int amt, uint8_t type = 0)
    {
        if(m_contents[m_activeSlot]->GetReserveAmmo() < m_contents[m_activeSlot]->GetData().iMaxAmmo ){
            m_contents[m_activeSlot]->GainAmmo(amt);
        }
    }
    void OnUpdate(){
        if(Active() == nullptr)
            return Error("active weapon bad reference in slot %d", m_activeSlot);
        Active()->OnUpdate();
       
    }
    void OnCreate(){
        
    }
private:
    uint8_t m_activeSlot;
    uint8_t m_filledSlots;
    std::array<CBaseWeapon*, INV_SLOTS> m_contents;
};

using  inventory_t = CInventory<SMITH_PLAYER_INV_SLOTS>;

