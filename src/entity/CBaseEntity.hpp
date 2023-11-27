#pragma once

#include <common.hpp>
#include <types/Vector.hpp>
class CBaseEntity
{
public:
    CBaseEntity() {}
    CBaseEntity(int m_iID) : m_iID(m_iID) {}
    virtual ~CBaseEntity(){}
    const auto GetID() { return m_iID; }
    const auto GetPosition() { return m_vecPosition; }
    virtual bool IsRenderable() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnCreate() = 0;
    virtual void OnDestroy() = 0;

protected:
    Vector m_vecPosition;
    uint32_t m_iID;
};