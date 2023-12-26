#include <light/CLight.hpp>

#include <interfaces/ILightingSystem/LightRegistry.hpp>
class CLightSun  : public CLight
{
public:
    CLightSun() : CLight(this) {}
    virtual ~CLightSun() {}
    virtual Color CalculateInfluence(const Vector& point, Color& color_in, const light_params& params, const Color& MaxDark)
    {
        return CLight::CalculateInfluenceBase(this, point, color_in, params, MaxDark);
    }
     virtual bool IsStatic() const {
        return true;
    }
private:
    REGISTER_DEC_LIGHT(CLightSun);

};