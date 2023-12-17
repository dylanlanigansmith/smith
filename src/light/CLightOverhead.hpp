#pragma once
#include <light/CLight.hpp>

#include <interfaces/ILightingSystem/LightRegistry.hpp>
class CLightOverhead  : public CLight
{
public:
    CLightOverhead() : CLight(this) {}
    virtual ~CLightOverhead() {}
    virtual Color CalculateInfluence(const Vector& point, Color& color_in, const light_params& params, const Color& MaxDark)
    {
        double distance = ( point - GetPosition() ).Length3D();
        float attenuation = 1.0f / (1.0f + params.a * distance + params.b * distance * distance);
        attenuation = (1.f - std::clamp(attenuation, params.minIntensity, 1.0f)) * params.rolloff;

        Color lightColor = GetColor();
        float brightness =  (1.f - GetIntensity()) * attenuation;
        float alphaFactor = ( brightness) * params.intensityModifier;
        lightColor.a(static_cast<uint8_t>(MaxDark.a() * alphaFactor));

        if(color_in.a() == MaxDark.a())
            color_in.a(lightColor.a());
        Color ret = lightColor + color_in;
        if(ret.r() == 45 && ret.a() == 0){
          //  gLog("out: %s in: %s", lightColor.s().c_str(), color_in.s().c_str());
        }

        return ret;
    }
private:
    REGISTER_DEC_LIGHT(CLightOverhead);

};