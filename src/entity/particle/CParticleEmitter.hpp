#pragma once

#include <entity/CBaseRenderable.hpp>
#include ENTREG_INC

class CParticleEmitter : public CBaseRenderable
{
public:
    CParticleEmitter(int m_iID) : CBaseRenderable(m_iID) {}

    virtual void OnUpdate() {}
    virtual void OnCreate();
    virtual void OnDestroy() {}
    virtual void CreateRenderable();
    virtual void OnRenderStart(){}
    virtual void OnRenderEnd(){}
    virtual void Render(CRenderer* renderer);

    virtual bool IsBlocking() const { return false; }
    virtual float GetBounds() const { return 0.1f; }
    virtual bool HasTexture() { return false; }
protected:
    virtual void SetupSystem();

protected:
    sprite_draw_data draw;
    IVector2 m_lastDrawStart;
    std::array<IVector2, 128> m_particles;
private:
    REGISTER_DEC_ENT(CParticleEmitter);
};