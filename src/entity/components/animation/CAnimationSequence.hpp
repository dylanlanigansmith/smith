#pragma once
#include <common.hpp>
#include <types/Vector.hpp>
#include <util/hash_fnv1a.hpp>
#include <SDL3/SDL.h>

#define HSEQUENCE_INVALID (hSequence)(-1)
typedef uint32_t hSequence;
namespace Anim
{
    static constexpr hSequence GetSequenceHandle(const char* name){
        return Util::fnv1a::hash_32_fnv1a_const(name);
    }

}


struct sequence_frame
{
    uint8_t num;
    SDL_Rect rect;
    IVector2 pos;
    sequence_frame(SDL_Rect rect, uint8_t num) : num(num), rect(rect) { pos = {-1,-1};}
    sequence_frame(SDL_Rect rect, uint8_t num, IVector2 pos) : num(num), rect(rect),  pos(pos) { }
};


class CAnimSequence
{
public:
    CAnimSequence(const std::string& m_szName) : m_szName(m_szName){
        m_hSequence = Anim::GetSequenceHandle(m_szName.c_str()); m_curFrame = 0; m_frameTime = 30;
    }
    CAnimSequence(const std::string& m_szName, uint32_t m_frameTime, const std::vector<sequence_frame>& frames)
     :  m_frames(frames),  m_frameTime(m_frameTime), m_szName(m_szName) {
        m_hSequence = Anim::GetSequenceHandle(m_szName.c_str()); m_curFrame = 0; 
    }
    hSequence handle() const { return m_hSequence; } 
    std::string name() const { return m_szName; }
    
    sequence_frame* at(uint8_t f) { handleFrameNum(f); return &m_frames.at(f); }
    void startframe() { handleFrameStart();}
    void endframe() { handleFrameEnd(); }
    sequence_frame* cur() { return &m_frames.at(m_curFrame); }
    sequence_frame* reset() { m_curFrame = 0 ; return &m_frames.at(m_curFrame); }

    bool last() { return m_curFrame >= (m_frames.size() - 1);}

    std::vector<sequence_frame> m_frames;
    uint8_t m_curFrame;
    uint32_t m_frameTime;
private:
    void handleFrameStart(){
        if(size_t(m_curFrame) >= m_frames.size())
            m_curFrame = 0;
    }
    void handleFrameEnd(){
            m_curFrame++;
    }
    void handleFrameNum(uint8_t f){
        assert(0 <= f && f < m_frames.size());
    }
    const std::string m_szName;
    hSequence m_hSequence;

};