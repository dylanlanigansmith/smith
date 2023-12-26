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
        return CLight::CalculateInfluenceBase(this, point, color_in, params, MaxDark);
    }
     virtual bool IsStatic() const {
        return true;
    }
private:
    REGISTER_DEC_LIGHT(CLightOverhead);

};