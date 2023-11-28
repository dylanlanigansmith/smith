#include "ITextureSystem.hpp"
#include "engine/engine.hpp"
CTextureSystem::~CTextureSystem()
{
}

void CTextureSystem::OnCreate()
{
    


    int imgFlags = IMG_INIT_PNG;
    if(!(IMG_Init(imgFlags) & imgFlags)){
        log("failed to init SDL3Img");
    }
    static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");
    m_szTextureResourcePath = IResourceSystem->GetResourceSubDir("material");
   
}

void CTextureSystem::OnShutdown()
{
    for(auto surf : texture_db)
    {
        SDL_DestroySurface(surf);
    }
}

hTexture CTextureSystem::LoadTexture(const std::string &name)
{
    SDL_Surface* load = NULL;
    SDL_DestroySurface(load); //idk the example did this

    load = IMG_Load(TextureNameToFile(name).c_str());
    if(load == NULL){
        log("failed loading texture %s !", name.c_str()); return HTEXTURE_INVALID;
    }
    SDL_Surface* optimized;
    SDL_DestroySurface(optimized);
    optimized = SDL_ConvertSurfaceFormat(load, SDL_PIXELFORMAT_RGBA8888);
    texture_db.push_back(optimized);
    SDL_DestroySurface(load);
    hTexture handle = texture_db.size() - 1;
    log("added %s to database @ %u", name.c_str(), handle);

    return handle;

}

SDL_Surface *CTextureSystem::GetTexture(hTexture handle)
{
    if(handle == HTEXTURE_INVALID || handle > texture_db.size() - 1){
        log("invalid handle %u not found in texture db [size: %i]", handle, texture_db.size()); return NULL;
    }
    SDL_Surface* gotTexture = texture_db.at(handle);
    if(gotTexture == NULL)
        log("found texture at %u but it is null?", handle); 
    return gotTexture;
    //https://lazyfoo.net/tutorials/SDL/06_extension_libraries_and_loading_other_image_formats/index2.php
}

std::string CTextureSystem::TextureNameToFile(const std::string &name)
{
    std::string full_path = m_szTextureResourcePath + name;
    return full_path;
}


