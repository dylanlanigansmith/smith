#pragma once

#include <common.hpp>
#include <types/Vector.hpp>
#include <util/rtti.hpp>
#include <types/Color.hpp>
#include <magic_enum/magic_enum.hpp>
#include <logger/logger.hpp>
enum class LightTypes : uint32_t //this is a bit of a hack buttt
{
    CLight = 0,
    CLightOverhead,
    CLightSun,
    LightTypesSize,
};

struct light_influence
{
    IVector2 min;
    IVector2 max;
};



class CLight
{
friend class CLightingSystem; friend class CEditor;
public:
    CLight() :  m_flIntensity(1.0f), m_flRange(1.0f), m_flBrightness(1.0f), m_iType(LightTypes::CLight), m_szName("CLight") {}
    template <typename T> 
    CLight(T* ptr ) : CLight() {
        m_szName = Util::getClassName<T>(ptr);

        auto find_type = magic_enum::enum_cast<LightTypes>(m_szName);
        if(find_type.has_value()){
            m_iType = find_type.value();
           // gLog("found %u from %s", m_iType, m_szName.c_str());
        }
        else{
           gError("Could not deduce lighttype %s", m_szName.c_str());
            m_iType = LightTypes::CLight;
        }
         
    }
    virtual ~CLight(){}



    //debug
    std::vector<std::tuple<Vector2, Vector2, bool>> rays;



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

    virtual void SetType(LightTypes type) { m_iType = type; }
    virtual LightTypes GetType() const { return m_iType; }

    virtual std::string GetName() const { return m_szName; }

private:
    Color m_color;
    float m_flIntensity;
    float m_flRange;
    float m_flBrightness;

    Vector m_vecPosition;
    LightTypes m_iType;
    std::string m_szName;
};
