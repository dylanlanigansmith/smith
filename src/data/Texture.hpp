#pragma once
#include <common.hpp>

#include <types/Vector.hpp>
#include <SDL3/SDL.h>
#include "CBaseSerializable.hpp"
#include <util/rtti.hpp>

#define ERROR_TEXTURE_SIZE IVector2(64,64)

#define HTEXTURE_INVALID (uint32_t)-1
typedef uint32_t hTexture;

struct texture_t 
{
    hTexture m_handle;
    IVector2 m_size;
    SDL_Surface* m_texture;
    uint32_t m_clrKey{};
    float m_flShade;
    uint32_t m_nMode{};

    texture_t() : m_handle(HTEXTURE_INVALID), m_size(ERROR_TEXTURE_SIZE),  m_texture(NULL),  m_flShade(1.0f) {}
    texture_t(const texture_t& t) : m_handle(t.m_handle), m_size(t.m_size), m_texture(t.m_texture), m_clrKey(t.m_clrKey),
            m_flShade(t.m_flShade), m_nMode(t.m_nMode) {}
    texture_t(const texture_t* t) : m_handle(t->m_handle), m_size(t->m_size), m_texture(t->m_texture), m_clrKey(t->m_clrKey),
            m_flShade(t->m_flShade), m_nMode(t->m_nMode) {}
    texture_t(hTexture m_handle, SDL_Surface* m_texture) : m_handle(m_handle), m_texture(m_texture), m_flShade(1.0f) { 
        if(m_texture == NULL){
            m_handle = HTEXTURE_INVALID; return;  }
        m_size = { m_texture->w, m_texture->h };
    }
    uint32_t getColorAtPoint(IVector2 t){
        return getColorAtPoint(t.x, t.y);
    }
    uint32_t getColorAtPoint(int x, int y){
        if(m_texture == NULL) return 0;
         uint32_t *pixelsT = (uint32_t *)m_texture->pixels;
        return pixelsT[(m_texture->pitch / 4 * y) + x]; // ABGR
    }

    bool SetColorAtPoint(int x, int y, uint32_t color){
        if(m_texture == NULL) return false;
        if(SDL_MUSTLOCK(m_texture)) return false;
        uint32_t *pixelsT = (uint32_t *)m_texture->pixels;
        pixelsT[(m_texture->pitch / 4 * y) + x] =  color;
        return true;
    }

    bool isTransparent(){
        return m_clrKey > 0u;
    }
};

class CTexture : public CBaseSerializable, public texture_t
{
public:
    CTexture() : CBaseSerializable(Util::getClassName(this)) { }
    CTexture(texture_t* t) : CBaseSerializable(Util::getClassName(this)), texture_t(t) { }
    virtual ~CTexture() {}

    virtual json ToJSON();
    virtual bool FromJSON(const json& j );
    virtual bool Validate();

    texture_t Data() const; 
    std::string m_szName;
};

