#pragma once
#include <common.hpp>
#include <data/Texture.hpp>
#include <imgui.h>
#include <renderer/renderer.hpp>
struct editor_texture_t
{
    std::string subpath = std::string();
    SDL_Texture* texture_preview = NULL;
    texture_t* texture = nullptr;
    ImVec4 primary_color;
};


struct dev_tools
{
    bool ent_info = false;
    bool fps = true;
    bool show_cam = false;
    bool show_pos = true;
};


class CEditor
{
public:
    static CEditor& instance(){
        static CEditor ce;
        return ce;
    }
    void render(CRenderer *renderer);
    bool isOpen() const { return m_bIsOpen; }
private:
    void drawMapView();
    void drawEntityView();
    void drawResourceView();
    void drawMaterialView();
    void Init();
private:
    void InitTextureInfo();
    void TexturePicker(const char* title, texture_t*& selectedTexture, SDL_Texture*& previewTexture, std::string& preview, ImGuiTextFilter* filter);


private:
    texture_t* m_texLastSelected = nullptr;
    bool m_bIsOpen = false;
    dev_tools settings;
    bool m_bHasInit = false;
    SDL_Renderer* m_renderer;
    std::unordered_map<std::string, editor_texture_t> texture_info;
   // std::unordered_map<texture_t*, SDL_Texture*> texture_previews; 
};