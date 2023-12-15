#include "CImageLoader.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <engine/engine.hpp>
#include <magic_enum/magic_enum.hpp>

int CImageLoader::Load(const std::string &img_path, SDL_Surface **surf)
{
    Debug(false);
    static auto IFileSystem = engine->CreateInterface<CFileSystem>("IFileSystem");
    if (!IFileSystem->FileExists(img_path))
    {
        Error("Image %s doesn't exist", img_path.c_str());
        return 1;
    }
    if (IFileSystem->GetExtension(img_path).find(".png") == std::string::npos)
    {
        Error("Image has extension %s, want '.png' ", IFileSystem->GetExtension(img_path).c_str());
        return 1;
    }

    // auto size = FindPNGSize(img_path);
    // if(size.x == 0 || size.y == 0){
    //     Error("failed to find img size! %d %d", size.x, size.y); return 1;
    //}

    // dbg("dims of %s are {%dx%d}",img_path.c_str(), size.x, size.y);
    int width, height, channels;
    unsigned char *img = stbi_load(img_path.c_str(), &width, &height, &channels, 0);
    if (img == NULL)
    {
        Error("failed to load image %dx%d @ %s", width, height, img_path.c_str());
        return 1;
    }
    int format = SDL_PIXELFORMAT_UNKNOWN;
    if (channels == 3)
    {
        format = SDL_PIXELFORMAT_RGB24;
    }
    else if (channels == 4)
    {
        format = SDL_PIXELFORMAT_RGBA32;
    }
    dbg("loaded image %dx%d @ %s", width, height, magic_enum::enum_name((SDL_PixelFormatEnum)format).data());
    *surf = SDL_CreateSurface(width, height, format);

    SDL_Surface *raw = SDL_CreateSurfaceFrom((void *)img, width, height, (*surf)->pitch, format);
    SDL_BlitSurface(raw, NULL, *surf, NULL);
    SDL_DestroySurface(raw);
    stbi_image_free(img);

    return 0;
}
