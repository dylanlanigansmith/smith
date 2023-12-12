#pragma once
#include <common.hpp>
#include <types/Vector.hpp>
#include <types/Color.hpp>
#include <nlohmann/json.hpp>
#include <global.hpp>
//#include <engine/engine.hpp>
#include <SDL3/SDL.h>
#include "Texture.hpp"

//using json =  nlohmann::json; 
struct anim_frame
{
    int m_index;
    SDL_Rect m_rect;
    IVector2 m_position;
};

class CAnimSurface
{
    public:
    CAnimSurface() {}
    CAnimSurface(const IVector2& size) : m_surf(NULL) {
        m_surf = SDL_CreateSurface(size.x, size.y, SMITH_PIXELFMT);
    }
    ~CAnimSurface(){
        SDL_DestroySurface(m_surf);
    }

    CAnimSurface(const CAnimSurface&) = delete;
    CAnimSurface& operator=(const CAnimSurface&) = delete;

    operator SDL_Surface*() {
        return m_surf;
    }
    inline Color getColorAtPoint(int x, int y){
        return ((uint32_t *)m_surf->pixels)[(m_surf->pitch / 4 * y) + x]; 
    }
    inline auto w() const { return m_surf->w ; }
    inline auto h() const { return m_surf->h ; }
    inline auto& pixels() { return m_surf->pixels ; }
    inline auto pitch() const { return m_surf->pitch ; }
private:
    SDL_Surface* m_surf;
};



inline nlohmann::json ivec2json(const IVector2& vec){
   using json =  nlohmann::json; 
    auto ret = json::array();
    ret = { vec.x, vec.y};
    return ret; 
}

class CAnimData
{
    friend class CEditor; friend class CAnimationSystem;
public:
    CAnimData() {}
    CAnimData(const std::string& m_szName) : m_szName(m_szName), m_numFrames(0) {}

    void AddFrame(int idx, const SDL_Rect& rect, const IVector2& pos = {0,0}){
        m_frames.push_back({idx, rect, pos}); m_numFrames++;
    }
    void AddFrame(){
        int idx = (m_frames.size() == 0) ? 0 : m_frames.size();

        m_frames.push_back({idx, {0,0,0,0}, {0,0}}); m_numFrames++;
    }

    
    auto& GetFrames() { return m_frames; }

    inline auto GetName() const { return m_szName; }
    inline auto GetSize() const { return m_size; }
    inline auto GetPosition() const { return m_pos; }
    inline auto GetRate() const { return m_rate; }
    inline auto GetMaskColor() const { return m_maskColor; }
    inline auto GetMaskColorAlt() const { return m_maskColorAlt; }
    inline auto GetTexture() const { return m_pTexture; }
    inline auto& GetSurface() { return m_pTexture->m_texture; }
    inline auto GetNumFrames() const { return m_numFrames; }

    inline int GetLastIndex() { return m_numFrames - 1; }
private:
    nlohmann::json ToJson(){
        using json =  nlohmann::json; 
        json  data = {
            m_szName, m_szTextureName, ivec2json(m_size), ivec2json(m_pos), m_rate, (uint32_t)m_maskColor, (uint32_t)m_maskColorAlt, m_numFrames 
        };

        json j = json::object();

        j.emplace("data", data);

        json fr = json::object();
        for(auto& frame : m_frames){
            json rect = {
                frame.m_rect.x, frame.m_rect.y, frame.m_rect.w, frame.m_rect.h
            };
            json fr_d = json::array(); fr_d = {
                frame.m_index, rect, ivec2json(frame.m_position)
            };
            fr.emplace(std::to_string(frame.m_index), fr_d);
        }
        j.emplace("frames", fr);

        //json joined = {{m_szName, j}};
        return j;
    }

    void FromJson(const nlohmann::json& j)
    {
        using json =  nlohmann::json; 

        json data = j["data"];
        json frames = j["frames"];

        m_szName = data.at(0);
        m_szTextureName = data.at(1);
        m_size = { (int) data.at(2).at(0), (int) data.at(2).at(1)};
        m_pos = { (int) data.at(3).at(0), (int) data.at(3).at(1)};
        m_rate = data.at(4);
        m_maskColor = Color((uint32_t)data.at(5));
        m_maskColorAlt = Color((uint32_t)data.at(6));
        m_numFrames = (size_t)data.at(7);

        m_frames.clear();
        for(auto& frame : frames){
            anim_frame af;
            af.m_index = frame.at(0);
            json r = frame.at(1);
            af.m_rect = {
                r.at(0), r.at(1), r.at(2), r.at(3)
            };
            af.m_position = {(int)frame.at(2).at(0), (int)frame.at(2).at(0)};
            m_frames.push_back(af);
        }
        assert(m_frames.size() == m_numFrames);

    }


private:
    std::string m_szName;
    std::string m_szTextureName;
    IVector2 m_size;
    IVector2 m_pos;
    int m_rate;

    size_t m_numFrames;
    std::vector<anim_frame> m_frames;
    
    Color m_maskColor;
    Color m_maskColorAlt;
    texture_t* m_pTexture;
};