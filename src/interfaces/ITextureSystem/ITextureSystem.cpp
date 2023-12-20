#include "ITextureSystem.hpp"
#include "engine/engine.hpp"
#include <global.hpp>
#include <util/hash_fnv1a.hpp>
#include <data/image/CImageLoader.hpp>

CTextureSystem::~CTextureSystem()
{
}

void CTextureSystem::OnCreate()
{
    Debug(false);
    m_hTextureError = HTEXTURE_INVALID;

    m_szTextureResourcePath = IResourceSystem->GetResourceSubDir("material");

    IResourceSystem->LoadTextureDefinition();
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
   // IMG_Quit();
}

void CTextureSystem::OnLoopStart()
{

   // IResourceSystem->SaveTextureDefinition();
}

SDL_Surface* CTextureSystem::LoadAndOptimizeSurface(const std::string& path, uint32_t key)
{
    
    static CImageLoader img_loader;
   

    SDL_Surface *load = NULL;
    SDL_DestroySurface(load); // idk the example did this

    if(img_loader.Load(path, &load) != 0){
         warn("imgloader failed with texture file %s !", path.c_str());
           return NULL;
    }
  

    if (load == NULL){
          warn("imgloader returned null for file %s !", path.c_str());
           return NULL;
      }
   
    SDL_Surface *optimized = NULL;

     optimized = SDL_ConvertSurfaceFormat(load, SMITH_PIXELFMT);
 
   
    if(key != 0u){
        SDL_SetSurfaceColorKey(optimized, SDL_TRUE, key);
    }

    SDL_DestroySurface(load);
    if(optimized != NULL)
        return optimized;

    warn("converting surface fmt failed: %s", SDL_GetError());
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

    new_texture->m_size = {new_texture->m_texture->w, new_texture->m_texture->h};
    return handle;
}

bool CTextureSystem::LoadFromDefinition(const CTexture& def)
{
    auto resource = TextureNameToFile(def.m_szName);
    if(resource.empty()) return false;
    

    auto optimized = LoadAndOptimizeSurface(resource, def.m_clrKey);
    if(optimized == NULL) return false;

    hTexture handle = GenerateHandle(def.m_szName);
    if(def.m_handle != handle){
        Error("handle def. %x does not match gen %x for %s", def.m_handle, handle, def.m_szName.c_str()); return false;
    }

    texture_t* new_texture = new texture_t(def.Data());
    new_texture->m_texture = optimized;
     new_texture->m_size = {new_texture->m_texture->w, new_texture->m_texture->h};
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
    dbg("finding or creating %s ", name.c_str());
    auto search = FindTexture(name);
    if(search != ErrorTextureHandle()){
        dbg("found %s %x", name.c_str(), search);   
        return GetTextureData(search);
    }
        
    
    auto hAdded = LoadTexture(name);
    if(!IsHandleValid(hAdded))
        Error("could not findorcreate texture %s, tried loading", name.c_str());
    log("findorcreate: created texture %s, %x", name.c_str(), hAdded);
    return GetTextureData(hAdded);
}

texture_t *CTextureSystem::GetTexture(hTexture handle)
{
   
    return GetTextureData(handle);
 
}

texture_t* CTextureSystem::GetTextureData(hTexture handle)
{
     if (!IsHandleValid(handle))
    {
        Error("Invalid Handle Requested %x", handle);
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


    
    std::string full_path = IResourceSystem->FindResource(m_szTextureResourcePath, name);
    return full_path;
}


