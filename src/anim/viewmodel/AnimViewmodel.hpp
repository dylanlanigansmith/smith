#pragma once
#include "../Animation.hpp"
#include <renderer/renderer.hpp>


struct flash_info
{
    voxel_t* lastVox;
    Color lastColor;
};

class CAnimViewmodel : public CAnimation
{
public:
    CAnimViewmodel(CBaseRenderable* m_pOwner, const std::string& name) : CAnimation(m_pOwner, name) {

        assert(m_pOwner->IsLocalPlayer());

    }


    void Draw(CRenderer* renderer, const IVector2& screen_pos, uint8_t alpha = 255);
protected:
    virtual void OnSequenceStart(const std::string& seq_name);
    virtual void OnSequenceEnd(const std::string& seq_name);
    flash_info m_flash;
};