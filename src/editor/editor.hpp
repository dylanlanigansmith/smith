#pragma once
#include <common.hpp>
#include <data/Texture.hpp>
#include <imgui.h>
#include <renderer/renderer.hpp>

#include <entity/dynamic/CBaseEnemy.hpp>
#include <entity/player/CPlayer.hpp>

#define UI_W 1680
#define UI_H 1050

struct editor_texture_t
{
    std::string subpath = std::string();
    SDL_Texture* texture_preview = NULL;
    texture_t* texture = nullptr;
    ImVec4 primary_color;
};


struct dev_tools
{
    bool ent_info = true;
    bool fps = true;
    bool show_cam = true;
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
    void drawNewMapEdit();
    void drawEntityView();
    void drawResourceView();
    void drawMaterialView();
    void drawMaterialEditor();
    void drawLightView();
    void drawSystemView();
    void drawAnimView();

    void drawSoundView();
    void Init();
private:
    void InitTextureInfo();
    void TexturePicker(const char *title, texture_t *&selectedTexture,
                            SDL_Texture *&previewTexture, std::string &preview, ImGuiTextFilter *filter);
    void TexturePicker(const char* title,tile_t* selectedTile, texture_t*& selectedTexture, SDL_Texture*& previewTexture, std::string& preview, ImGuiTextFilter* filter, uint8_t updatetype = 0);
    void ShowEntityObject(CBaseEntity* entity, ImVec2 offset, ImDrawList *draw_list, tile_t* lastTile = nullptr);

private:
    texture_t* m_texLastSelected = nullptr;
    bool m_bIsOpen = false;
    dev_tools settings;
    bool m_bHasInit = false;
    CRenderer * smith_renderer;
    SDL_Renderer* m_renderer;
    std::unordered_map<std::string, editor_texture_t> texture_info;
   // std::unordered_map<texture_t*, SDL_Texture*> texture_previews; 
};