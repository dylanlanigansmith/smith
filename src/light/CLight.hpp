#pragma once

#include <common.hpp>
#include <types/Vector.hpp>
#include <util/rtti.hpp>
#include <types/Color.hpp>
#include <magic_enum/magic_enum.hpp>
#include <logger/logger.hpp>

struct light_influence
{
    IVector2 min;
    IVector2 max;
};

struct light_params
{
    float a = 0.4f;
    float b =  0.095; //1.81f;
    float minIntensity = 0.01;
    float alphaMod = 1.0f;
    float colorModifier = 1.0f;
    float finalModifier = 1.0f;
    float rolloff = 1.0f;
    float intensityModifier = 1.0f;
    float interpFraction = 0.41f;
    int method = 0; // to be added later
    bool neighbor_interp = false;
    bool use_global = true;
    bool dynamic = false;
};

class CLight
{
friend class CLightingSystem; friend class CEditor;
public:
    CLight() :  m_flIntensity(1.0f), m_flRange(1.0f), m_flBrightness(1.0f), m_szName("CLight") {}
    template <typename T> 
    CLight(T* ptr ) : CLight() {
        m_szName = Util::getClassName<T>(ptr);

     
         
    }
    virtual ~CLight(){}



    //debug
    std::vector<std::tuple<Vector2, Vector2, bool>> rays;

    virtual Color CalculateInfluence(const Vector& point, Color& color_in, const light_params& params, const Color& MaxDark ) = 0;

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

 

    virtual std::string GetName() const { return m_szName; }


    static inline Color CalculateInfluenceBase(CLight* light, const Vector& point, Color& color_in, const light_params& params, const Color& MaxDark)
    {
        double distance = ( point - light->GetPosition() ).Length3D();
        float attenuation = 1.0f / (1.0f + params.a * distance + params.b * distance * distance);
        attenuation = (1.f - std::clamp(attenuation, params.minIntensity, 1.0f)) * params.rolloff;

        Color lightColor = light->GetColor();
        float brightness =  (1.f - light->GetIntensity()) * attenuation;
        float alphaFactor = ( brightness) * params.intensityModifier;
        lightColor.a(static_cast<uint8_t>(MaxDark.a() * alphaFactor));

        if(color_in.a() == MaxDark.a())
            color_in.a(lightColor.a());
        return lightColor + color_in;
    }

    virtual void OnUpdate() {}

    virtual bool IsStatic() const = 0;


    inline void AddInfluence(const Vector& pt){
        m_influence.insert(pt);
    }

    auto& GetInfluence() { return m_influence; }
    virtual void Reset() { m_influence.clear(); }
    auto InfluenceSize() { return m_influence.size(); }

    virtual bool IsTemporary() const { return false; }
protected:
    Color m_color;
    float m_flIntensity;
    float m_flRange;
    float m_flBrightness;
    std::unordered_set<Vector> m_influence;
  //  std::vector<std::vector<std::vector<Color> > > m_original;
    Vector m_vecPosition;


    std::string m_szName;


};
