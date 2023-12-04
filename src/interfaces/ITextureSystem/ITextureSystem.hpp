#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <data/Texture.hpp>


class CTextureSystem : public CBaseInterface
{
public:
friend class CResourceSystem;
friend class CEditor;
    CTextureSystem() : CBaseInterface("ITextureSystem") { }
    ~CTextureSystem() override;
    virtual void OnCreate() override;
    virtual void OnShutdown() override;
    virtual void OnLoopStart() override; 
    virtual void OnLoopEnd() override {}
    virtual void OnRenderStart() override {}
    virtual void OnRenderEnd() override {}
    virtual void OnEngineInitFinish() override;
    virtual void OnResourceLoadEnd() override;


    virtual hTexture LoadTexture(const std::string& name);
    virtual bool LoadFromDefinition(const CTexture& def);
    virtual hTexture FindTexture(const std::string& name);
    virtual texture_t* FindOrCreatetexture(const std::string& name);
    virtual texture_t* GetTexture(hTexture handle); //need to phase this out
    virtual texture_t* GetTextureData(hTexture handle);
    virtual hTexture ErrorTextureHandle();
    virtual texture_t* ErrorTexture();
    virtual bool IsHandleValid(hTexture handle);
    virtual void GetTextureSize(int* w, int* h);
    virtual void GetTextureSize(hTexture handle, int* w, int* h);
    virtual const std::string FilenameFromHandle(hTexture handle); 
    //FindOrCreateTexture
private:
    std::string TextureNameToFile(const std::string& name);
    hTexture GenerateHandle(const std::string& name);
    SDL_Surface* LoadAndOptimizeSurface(const std::string& path, uint32_t key = 0);
    bool AddTexture(const std::string& name, texture_t* text, bool log_add = true);
private:
    hTexture m_hTextureError;
    texture_t* m_textureError;
    std::string m_szTextureResourcePath;
    std::unordered_map<hTexture, texture_t*> texture_db;
     std::unordered_map<hTexture, std::string> texture_lookup;
};
