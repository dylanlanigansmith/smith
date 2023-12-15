#pragma once
#include "../Animation.hpp"
#include <renderer/renderer.hpp>
#include <entity/CBaseRenderable.hpp>



enum AnimDirections : int //not flipped
{
    AnimDir_Facing = 0,
    AnimDir_Side,
    AnimDir_DiagFacing,
    AnimDir_DiagAway,
    AnimDir_Away
};
enum AnimDir_States : int {
    AnimDir_Default = 0,
    AnimDir_Walking,
    AnimDir_Attacking,
    AnimDir_Dying,
    AnimDir_Dead,
    AnimDir_Standing = AnimDir_Default
};

enum AnimDir_Flip : int 
{
    AnimDir_NoFlip = 0,
    AnimDir_FlipH,
    AnimDir_FlipV
};


 using orient_callback_fn = std::function<int(int*,int*,int*, std::string& )>;


class CAnimDirectional : public CAnimation
{
   
public:
    enum AnimOrientations : int //http://www.doomlegends.com/emporium/tutorials/spangles.gif
    {
        DefaultOrientation = 1,
        Dead = 0,
        Facing = 1,
        Face_DiagLeft = 2,
        Face_Left = 3,
        Away_DiagLeft = 4,
        Away = 5,
        Away_DiagRight = 6,
        Face_Right = 7,
        Face_DiagRight = 8,
        Face_Dying = 9
    };

public:
    CAnimDirectional(CBaseRenderable* m_pOwner, const std::string& name) : CAnimation(m_pOwner, name) {
        Debug(false);
    }

    virtual void OnCreate();
    virtual void OnUpdate();
    virtual void Draw(CRenderer* renderer, const sprite_draw_data& data );

    void SetCallback(orient_callback_fn fn) { DeduceOrientationCallback = fn;}

    virtual void ChangeBaseTexture(const std::string& texture_name);

     virtual uint32_t GetPixelAtPoint(const IVector2 &point, IVector2* textpos, const sprite_draw_data& data);
     auto GetCurrentSeqName() const { return m_currentlyPlaying;}
protected:
    virtual void OnSequenceStart(const std::string& seq_name);
    virtual void OnSequenceEnd(const std::string& seq_name);
    
protected:
    orient_callback_fn DeduceOrientationCallback; //returns orientation, sets flip, animstate, seq_name
    int m_playingForState = 0;
    std::string m_currentlyPlaying;
    
    int m_orientation = DefaultOrientation;
    int m_direction = 0;
    int m_animstate = 0;
    int m_animflip = 0;
};