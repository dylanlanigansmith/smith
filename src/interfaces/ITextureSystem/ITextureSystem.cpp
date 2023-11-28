#include "ITextureSystem.hpp"
#include "engine/engine.hpp"
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
    static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");
    m_szTextureResourcePath = IResourceSystem->GetResourceSubDir("material");
}

void CTextureSystem::OnShutdown()
{
    for (auto surf : texture_db)
    {
        SDL_DestroySurface(surf);
    }
}

hTexture CTextureSystem::LoadTexture(const std::string &name)
{
    SDL_Surface *load = NULL;
    SDL_DestroySurface(load); // idk the example did this

    load = IMG_Load(TextureNameToFile(name).c_str());
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
    if (load == NULL)
    {
        log("failed loading texture %s !", name.c_str());
        return HTEXTURE_INVALID;
    }
    SDL_Surface *optimized;
    //  SDL_DestroySurface(optimized);
    optimized = SDL_ConvertSurfaceFormat(load, SDL_PIXELFORMAT_RGBA8888);
    texture_db.push_back(optimized);
    SDL_DestroySurface(load);
    hTexture handle = texture_db.size() - 1;
    log("added %s to database @ %u", name.c_str(), handle);

    return handle;
}

SDL_Surface *CTextureSystem::GetTexture(hTexture handle)
{
    if (!IsHandleValid(handle))
    {
        return NULL;
    }
    SDL_Surface *gotTexture = texture_db.at(handle);
    if (gotTexture == NULL)
        log("found texture at %u but it is null?", handle);
    return gotTexture;
    // https://lazyfoo.net/tutorials/SDL/06_extension_libraries_and_loading_other_image_formats/index2.php
}

bool CTextureSystem::IsHandleValid(hTexture handle)
{
    if (handle == HTEXTURE_INVALID || handle > texture_db.size() - 1)
    {
        log("invalid handle %u not found in texture db [size: %i]", handle, texture_db.size());
        return false;
    }
    return true;
}

void CTextureSystem::GetTextureSize(int *w, int *h)
{
    auto text = GetTexture(ErrorTexture());
    *w = text->w;
    *h = text->h;
}

std::string CTextureSystem::TextureNameToFile(const std::string &name)
{
    std::string full_path = m_szTextureResourcePath + name;
    return full_path;
}
