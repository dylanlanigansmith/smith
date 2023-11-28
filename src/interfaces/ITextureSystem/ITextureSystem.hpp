#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#define HTEXTURE_INVALID -1
typedef uint32_t hTexture;

class CTextureSystem : public CBaseInterface
{
public:
    CTextureSystem() : CBaseInterface("ITextureSystem") { }
    ~CTextureSystem() override;
    virtual void OnCreate() override;
    virtual void OnShutdown() override;
    virtual void OnLoopStart() override {}
    virtual void OnLoopEnd() override {}
    virtual void OnRenderStart() override {}
    virtual void OnRenderEnd() override {}
    virtual hTexture LoadTexture(const std::string& name);
    virtual SDL_Surface* GetTexture(hTexture handle);
    virtual hTexture ErrorTexture() { return 0; }
    virtual bool IsHandleValid(hTexture handle);
    virtual void GetTextureSize(int* w, int* h);
    //FindOrCreateTexture
private:
    std::string TextureNameToFile(const std::string& name);
    
private:
    std::string m_szTextureResourcePath;
    std::vector<SDL_Surface*> texture_db;
};
