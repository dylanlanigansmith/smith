#include "ITextureSystem.hpp"
#include "engine/engine.hpp"
#include <global.hpp>
#include <util/hash_fnv1a.hpp>

CTextureSystem::~CTextureSystem()
{
}

void CTextureSystem::OnCreate()
{

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        log("failed to init SDL3Img");
    }
    m_hTextureError = HTEXTURE_INVALID;
    static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");
    m_szTextureResourcePath = IResourceSystem->GetResourceSubDir("material");
}

void CTextureSystem::OnResourceLoadEnd()
{
    m_hTextureError = FindTexture("error.png");
    assert(ErrorTextureHandle() != HTEXTURE_INVALID); //ensure this is loaded asap
    m_textureError = GetTextureData(m_hTextureError);
    assert(m_textureError != nullptr && m_textureError->m_texture != NULL);
}

void CTextureSystem::OnEngineInitFinish()
{
   
}


void CTextureSystem::OnShutdown()
{
    for (auto entry : texture_db)
    {
        auto texture = entry.second;
        //safe to pass null to destroysurface
        SDL_DestroySurface(texture->m_texture);
        delete texture; //bye
    }
}

void CTextureSystem::OnLoopStart()
{
  // static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");
   // IResourceSystem->SaveTextureDefinition();
}

SDL_Surface* CTextureSystem::LoadAndOptimizeSurface(const std::string& path)
{
     /*
    Load an image from a filesystem path into a software surface.
    An SDL_Surface is a buffer of pixels in memory accessible by the CPU.
     Use this if you plan to hand the data to something else or manipulate it further in code.
     There are no guarantees about what format the new SDL_Surface data will be; in many cases,
      SDL_image will attempt to supply a surface that exactly matches the provided image,
      but in others it might have to convert (either because the image is in a format that SDL doesn't directly support,
       or because it's compressed data that could reasonably uncompress to various formats and SDL_image had to pick one).
       You can inspect an SDL_Surface for its specifics, and use SDL_ConvertSurface to then migrate to any supported format.
       If the image format supports a transparent pixel, SDL will set the colorkey for the surface.
       You can enable RLE acceleration on the surface afterwards by calling: SDL_SetSurfaceColorKey(image, SDL_RLEACCEL, image->format->colorkey);
       There is a separate function to read files from an SDL_RWops,
       if you need an i/o abstraction to provide data from anywhere instead of a simple filesystem read;
       that function is IMG_Load_RW(). If you are using SDL's 2D rendering API,
       there is an equivalent call to load images directly into an SDL_Texture for use by the GPU without using a software surface:
        call IMG_LoadTexture() instead. When done with the returned surface, the app should dispose of it with a call to SDL_DestroySurface().
    */
    SDL_Surface *load = NULL;
    SDL_DestroySurface(load); // idk the example did this

    load = IMG_Load(path.c_str());
    if (load == NULL){
        log("failed loading texture file %s !", path.c_str());
        return NULL;
    }
    SDL_Surface *optimized = NULL;
    //  SDL_DestroySurface(optimized);
     optimized = SDL_ConvertSurfaceFormat(load, SMITH_PIXELFMT);
      // https://lazyfoo.net/tutorials/SDL/06_extension_libraries_and_loading_other_image_formats/index2.php

    SDL_DestroySurface(load);
    if(optimized != NULL)
        return optimized;

    log("converting surface fmt failed: %s", SDL_GetError());
    return NULL;
}

bool CTextureSystem::AddTexture(const std::string& name, texture_t* text, bool log_add)
{
    
    
    auto ins_look = texture_lookup.emplace(text->m_handle, name);
    auto ins_db = texture_db.emplace(text->m_handle, text);
    if(!ins_look.second || !ins_db.second){
        Error("failed to add %s [%ix%i] to database @ %x", name.c_str(),text->m_size.x, text->m_size.y, text->m_handle);
    }

    if(log_add)
        log("added %s [%ix%i] to database @ %x", name.c_str(),text->m_size.x, text->m_size.y, text->m_handle);
    else
        info("added %s [%ix%i] to database @ %x", name.c_str(),text->m_size.x, text->m_size.y, text->m_handle);
    return true;
}


hTexture CTextureSystem::LoadTexture(const std::string &name)
{
    auto resource = TextureNameToFile(name);
    if(resource.empty()) return HTEXTURE_INVALID;
 
    auto optimized = LoadAndOptimizeSurface(resource);
    if(optimized == NULL) return HTEXTURE_INVALID;

    hTexture handle = GenerateHandle(name);
    
    texture_t* new_texture = new texture_t(handle, optimized);

    if(!AddTexture(name, new_texture)) return HTEXTURE_INVALID;

    return handle;
}

bool CTextureSystem::LoadFromDefinition(const CTexture& def)
{
    auto resource = TextureNameToFile(def.m_szName);
    if(resource.empty()) return false;

    auto optimized = LoadAndOptimizeSurface(resource);
    if(optimized == NULL) return false;

    hTexture handle = GenerateHandle(def.m_szName);
    if(def.m_handle != handle){
        Error("handle def. %x does not match gen %x for %s", def.m_handle, handle, def.m_szName.c_str()); return false;
    }

    texture_t* new_texture = new texture_t(def.Data());
    new_texture->m_texture = optimized;
    if(!AddTexture(def.m_szName, new_texture, false)) return false;


    return true;

}

hTexture CTextureSystem::FindTexture(const std::string& name)
{

    auto search_handle = GenerateHandle(name);
    auto search = texture_db.find(search_handle);
    if(search != texture_db.end())
        return search_handle;

    auto it = std::find_if(std::begin(texture_lookup), std::end(texture_lookup),
                           [name](auto&& p) { return p.second == name; });

    if (it == texture_lookup.end())
        return ErrorTextureHandle();
    
    return it->first;
}

texture_t* CTextureSystem::FindOrCreatetexture(const std::string& name)
{
    auto search = FindTexture(name);
    if(search != ErrorTextureHandle())
        return GetTexture(search);
    
    auto hAdded = LoadTexture(name);
    if(!IsHandleValid(hAdded))
        Error("could not findorcreate texture %s", name.c_str());
    return GetTexture(hAdded);
}

texture_t *CTextureSystem::GetTexture(hTexture handle)
{
   
    return GetTextureData(handle);
 
}

texture_t* CTextureSystem::GetTextureData(hTexture handle)
{
     if (!IsHandleValid(handle))
    {
        return ErrorTexture(); //GetTexture(ErrorTexture());
    }
    texture_t* gotTexture =  nullptr;
    try
    {
        gotTexture = texture_db.at(handle);
    }
    catch(const std::exception& e) //std::outofrange
    {
        std::cerr << e.what() << '\n';
        Error("error with texture %x / %s", handle, FilenameFromHandle(handle).c_str());
    }
       
    return (gotTexture == nullptr) ? ErrorTexture() : gotTexture;
}



texture_t* CTextureSystem::ErrorTexture()
{
    return m_textureError;
}

hTexture CTextureSystem::ErrorTextureHandle()
{
    return m_hTextureError;
}

hTexture CTextureSystem::GenerateHandle(const std::string& name)
{
    return Util::fnv1a::Hash(name.c_str());
}

bool CTextureSystem::IsHandleValid(hTexture handle)
{
    if (handle == HTEXTURE_INVALID)
    {
        log("invalid handle %u requested", handle);
        return false;
    }
    return true;
}

void CTextureSystem::GetTextureSize(int *w, int *h)
{
    auto& size = ErrorTexture()->m_size;
     *w = size.x;
    *h = size.y;
}

void CTextureSystem::GetTextureSize(hTexture handle, int* w, int* h)
{
     auto& size = GetTexture(handle)->m_size;
    *w = size.x;
    *h = size.y;
}

const std::string CTextureSystem::FilenameFromHandle(hTexture handle)
{
    auto search = texture_lookup.find(handle);
    if(search != texture_lookup.end()){
        return texture_lookup.at(handle);
    }
    Error("could not find filename record for handle %x", handle);
    return std::to_string(handle);
}

std::string CTextureSystem::TextureNameToFile(const std::string &name)
{

    static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");
    
    std::string full_path = IResourceSystem->FindResource(m_szTextureResourcePath, name);
    return full_path;
}


