#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <SDL3/SDL.h>
#include <types/Vector.hpp>


class CLightingSystem : public CBaseInterface
{
public:
    CLightingSystem() : CBaseInterface("ILightingSystem") { }
    ~CLightingSystem() override {}
    virtual void OnCreate() override {}
    virtual void OnShutdown() override {}
    virtual void OnLoopStart() override {}
    virtual void OnLoopEnd() override {}
    virtual void OnRenderStart() override{}
    virtual void OnRenderEnd() override {}
 
private:
   
};

/*
So... do we do real time or baked
- bake for walls

-shadows for entities and props seem easy, walls not so much

Top of render loop calls Light sys -> calculate

then everything asks for its light value

for tiles:
every tile has  a light emission value
-can be from a face if it is a wall or from ceiling 
ray trace 0.5 step to tiles in radius 
difuse effect over 0.5 steps

lighting first then shadows i guess

*/