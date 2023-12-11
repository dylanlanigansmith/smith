#pragma once

#include <entity/weapon/CBaseWeapon.hpp>
#include <entity/weapon/pistol/CWeaponPistol.hpp>
#include "CInventory.hpp"
class CPlayer;

struct viewmodel_params
{
    int m_crosshairWidth;
    int m_crosshairLength;
    bool m_crosshairDot;
    bool m_crosshairGap;
    Color m_crosshairAltColor;
    Color m_crosshairColor;
    bool m_bShow;
    bool m_bUseLighting;

    viewmodel_params() : 
        m_crosshairWidth(1), m_crosshairLength(4), m_crosshairDot(false), m_crosshairGap(true), 
         m_crosshairAltColor(Color::Black()), m_crosshairColor(Color::White()), m_bShow(true), m_bUseLighting(true){}
};


class CViewmodel
{
    public:
    CViewmodel(CPlayer* m_pParent) : m_pParent(m_pParent){

    }
    void Setup(inventory_t* inv) { m_inventory = inv; }
    virtual void Render(CRenderer* renderer);

    auto& Settings() { return params; }
    private:
    void DrawCrosshair(CRenderer *renderer);
    CPlayer* Owner();
    inline CBaseWeapon* ActiveWeapon() { return m_inventory->Active(); }

     viewmodel_params params;
    uint8_t m_activeWeaponIndex;
    
    inventory_t* m_inventory;
    CPlayer* m_pParent;
};

/*

Player owns viewmodel and inventory, 
    ->inventory handles adding/removing/switching weapons, and runs their logic, 
    ->viewmodel handles rendering

Todo:
pickup dropped weapon
    -this means sprite collision detection
rewrite weapons for better universal base class
->add ammo
-> add more weapons

to add: 
-fists
-smg

fists should be able to press buttons and stuff, not attack tho



shared:
animation system redux
-using current for weapons is fine
-needs a branch for relative 2 sprite animations that are drawn together 

For sprites:
-should add a directional one 
-need animation system for shooting, hit animations 
-should use new collision detection for voxel based pathfinding
-scripted behaviour components

for player state and sprites:
-serialization
-(mostly sprites)

*/