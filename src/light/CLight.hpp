#pragma once

#include <common.hpp>
#include <types/Vector.hpp>
#include <util/rtti.hpp>
#include <types/Color.hpp>


struct light_influence
{
    IVector2 min;
    IVector2 max;
};


class CLight
{
friend class CLightingSystem;
public:
    CLight() :  m_flIntensity(1.0f), m_flRange(1.0f), m_flBrightness(1.0f), m_iType(0), m_szName("CLight") {}
    template <typename T> 
    CLight(T* ptr) : CLight() { m_szName = Util::getClassName<T>(ptr); }
    virtual ~CLight(){}







    virtual void SetPosition(const Vector& position) { m_vecPosition = position; }
    virtual Vector GetPosition() const { return m_vecPosition; }

    virtual void SetColor(const Color& color) { m_color = color; }
    virtual Color GetColor() const { return m_color; }

    virtual void SetIntensity(float intensity) { m_flIntensity = intensity; }
    virtual float GetIntensity() const { return m_flIntensity; }

    virtual void SetRange(float range) { m_flRange = range; }
    virtual float GetRange() const { return m_flRange; }

    virtual void SetBrightness(float brightness) { m_flBrightness = brightness; }
    virtual float GetBrightness() const { return m_flBrightness; }

    virtual void SetType(uint32_t type) { m_iType = type; }
    virtual uint32_t GetType() const { return m_iType; }

    virtual std::string GetName() const { return m_szName; }

private:
    Color m_color;
    float m_flIntensity;
    float m_flRange;
    float m_flBrightness;

    Vector m_vecPosition;
    uint32_t m_iType;
    std::string m_szName;
};
