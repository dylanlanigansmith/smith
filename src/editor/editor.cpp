#include "editor.hpp"
#include <engine/engine.hpp>

#include <imgui_impl_sdlrenderer3.h>
#include <imgui_impl_sdl3.h>
#include <SDL3/SDL.h>
#include "editor_helpers.hpp"
#include <renderer/render_helpers.hpp>
#define MENULOG(fmt, ...) engine->log(fmt, __VA_ARGS__)

void CEditor::Init()
{
    m_bHasInit = true;   
    
    auto bd = ImGui_ImplSDLRenderer3_GetBackendData();
    m_renderer = bd->SDLRenderer; 

    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
    
    for(const auto& entry : ITextureSystem->texture_lookup)
    {
        auto texture = ITextureSystem->GetTextureData(entry.first);

        int samplePt = texture->m_size.x / 2;
        SDL_Color clr = Render::TextureToSDLColor(texture->getColorAtPoint(samplePt, samplePt));

        
        editor_texture_t to_add {
            .subpath = "",
            .texture_preview = SDL_CreateTextureFromSurface(m_renderer, texture->m_texture),
            .texture = texture
        };
        texture_info.emplace(entry.second, to_add);

    }
    MENULOG("Editor Init %li", texture_info.size());
}

void CEditor::render()
{
    static bool changedLastFrame = false;
    static auto IInputSystem = engine->CreateInterface<CInputSystem>("IInputSystem");
    if(IInputSystem->IsKeyDown(SDL_SCANCODE_BACKSLASH) ){
        if(!changedLastFrame){
             m_bIsOpen = !m_bIsOpen;
            changedLastFrame = true;
        }
    }
    else changedLastFrame = false;
    if(!isOpen())
        return;
    SDL_SetRelativeMouseMode(SDL_FALSE);
    
    if(!m_bHasInit)
        Init();

     ImVec2 windowSize(1100,700);
    ImVec2 windowPos(0,0);
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("SmithEditor v0.0.0");
   // ImGui::ShowDemoWindow();
    //the code here can be a bit greasy because it is a dev tool but try not to make it too singletrack
    if(ImGui::BeginTabBar("###mode"))
    {
        if(ImGui::BeginTabItem("Map View"))
        {
            drawMapView();
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Resources"))
        {
            drawResourceView();
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Scene View"))
        {
             drawEntityView();
             /*
                gonna need to serialize entities :(
                    - either add tojson to CBaseEntity or a getserializer func that returns a custom CBaseSerializable sorta thing
                    - or just fuckin save classtype and write it as bytes and rip backwards compatibility.. 
                    - i see why code generators and shit exist now for big engines, like marking members as @saved or @networked etc 
                    - final option: CBaseSerializableField, template for types etc, might be fun, every field gets added to a CSerializeControllerComponent or soemthing
                        -tldr: entities havent been fleshed out yet and we can probably make this simple for static props at least (subclass!!)
                
             */
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
      ImGui::End();

   
}

void CEditor::drawMapView()
{
    //imgui demo 2366
    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");
    static tile_t* selectedTile = nullptr;
    auto& level = ILevelSystem->m_Level;
    auto& world = level->world;

   

    static std::string primary_texture_preview = "--";
    static texture_t* selectedTexture = nullptr;
    static SDL_Texture* previewTexture = NULL;


    ImGui::Columns(2, "###mapedit");
    ImVec2 tileSize(20,20);
    for(auto& row : world){
        for(auto& tile : row){
            ImGui::PushID(tile.id);
            
            auto color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
            if(selectedTile == &tile)
                color = ImVec4(color.x / 6.f, color.y /6.f, color.z / 6.f, 1.f);
            ImGui::PushStyleColor(ImGuiCol_Button, color);
            if(ImGui::Button(std::to_string(tile.m_nType).c_str(), tileSize)){
                selectedTile = &tile;
                selectedTexture = tile.m_pTexture;
                primary_texture_preview = ITextureSystem->FilenameFromHandle(selectedTexture->m_handle);
                previewTexture = texture_info.at(primary_texture_preview).texture_preview;
            }
            if( !(tile.m_vecPosition.x == MAP_SIZE - 1) ){
                ImGui::SameLine();
            }
            ImGui::PopStyleColor();
            ImGui::PopID();
        }
    }
    ImGui::NextColumn();
    if(ImGui::Button("Save")){
        IResourceSystem->SaveLevel();
    }
    if(selectedTile == nullptr){
        ImGui::Text("No Tile Selected");
        ImGui::Columns();
        return;
    }
    ImGui::Text("Selected Tile: {%i, %i}", selectedTile->m_vecPosition.x, selectedTile->m_vecPosition.y);
    
   

    
    if(ImGui::BeginCombo("Texture", primary_texture_preview.c_str(), 0))
    {
        for(const auto& entry : texture_info)
        {
            ImGui::PushID(&entry);
            std::string name = entry.first;
            if(ImGui::Selectable(name.c_str(), selectedTexture == entry.second.texture))
            {
                selectedTexture = entry.second.texture;
                primary_texture_preview = ITextureSystem->FilenameFromHandle(selectedTexture->m_handle);
               previewTexture = entry.second.texture_preview;
               
            }
            ImGui::SameLine();
             ImGui::Image(entry.second.texture_preview, ImVec2(32, 32));
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
   
    if(selectedTexture != nullptr && previewTexture != NULL)
    {
        ImGui::Image(previewTexture, ImVec2(64, 64));
        selectedTile->m_pTexture = selectedTexture; //time to finish porting texture system
    }
    

    ImGui::Columns();
    //Editor::IVec2Str(tile.m_vecPosition).c_str(),
  
}

void CEditor::drawEntityView()
{
    //there is an example for this in the imguidemo
     static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
}

void CEditor::drawResourceView()
{
    
}


