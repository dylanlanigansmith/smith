#include "editor.hpp"
#include <engine/engine.hpp>

#include <imgui_impl_sdlrenderer3.h>
#include <imgui_impl_sdl3.h>
#include <SDL3/SDL.h>
#include "editor_helpers.hpp"
#include <renderer/render_helpers.hpp>
#include <util/misc.hpp>
#define MENULOG(fmt, ...) engine->log(fmt, __VA_ARGS__)

void CEditor::Init()
{
    m_bHasInit = true;   
    
    auto bd = ImGui_ImplSDLRenderer3_GetBackendData();
    m_renderer = bd->SDLRenderer; 
    InitTextureInfo();
   
    MENULOG("Editor Init %li", texture_info.size());
}

void CEditor::InitTextureInfo()
{
     static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
    
    texture_info.clear(); //GIANT MEMORY LEAK!!
    for(const auto& entry : ITextureSystem->texture_lookup)
    {
        auto texture = ITextureSystem->GetTextureData(entry.first);
        editor_texture_t to_add {
            .subpath = "",
            .texture_preview = SDL_CreateTextureFromSurface(m_renderer, texture->m_texture),
            .texture = texture,
            .primary_color = Editor::SDLClrToImClr4( Editor::GenerateAverageColor(texture)),
        };
        texture_info.emplace(entry.second, to_add);

    }
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

    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
    static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
    auto player = IEntitySystem->GetLocalPlayer();
    if(auto draw = ImGui::GetBackgroundDrawList(); draw) //just for scope really
    {
        static constexpr auto text_color = IM_COL32(255,255,255,255);
        auto p = player->GetPosition();
        std::string str_playerpos = Util::stringf("(%.1f, %.1f, %.1f)", p.x, p.y, p.z);
        auto textSize = ImGui::CalcTextSize(str_playerpos.c_str());
        static const ImVec2 position(SCREEN_WIDTH - 10,  SCREEN_HEIGHT - 10);
        draw->AddText(position - textSize, text_color, str_playerpos.c_str());

        if(false)
        {
             auto c = player->Camera().m_vecPlane;
            auto d = player->Camera().m_vecDir;
            std::string str_camplane = Util::stringf("(%.1f, %.1f) | (%.1f, %.1f)", c.x, c.y, d.x,d.y);
            auto camtextSize = ImGui::CalcTextSize(str_camplane.c_str());
            static const ImVec2 camposition(SCREEN_WIDTH - 10,  SCREEN_HEIGHT - 25);
            draw->AddText(camposition - camtextSize, text_color, str_camplane.c_str());
        }
        static float fps_min = 10000.f;
        static float fps_max = 0.f;
        float fps =  IEngineTime->GetFPS(); 
        float fps_avg = IEngineTime->GetFPSAvg();
        if(fps < fps_min) fps_min = fps;
        if(fps > fps_max) fps_max = fps;
        if(fps_max > 10000) fps_max = 0;
        std::string fps_str = Util::stringf("%.2f | (%.1f, %.1f)",fps_avg, fps_min, fps_max);
        auto textSize_fps = ImGui::CalcTextSize(fps_str.c_str()) / 2; //textSize_fps.x =- 1.f;
         draw->AddText(textSize_fps, text_color, fps_str.c_str());
       
    }

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
    static std::string ceiling_texture_preview = "--";
    static std::string floor_texture_preview = "--";
    static std::string paint_texture_preview = "--";
    static const char* type_preview = "--";
    static texture_t* paintTexture = nullptr;
    static texture_t* selectedTexture = nullptr;
    static texture_t* selectedTextureFloor = nullptr;
    static texture_t* selectedTextureCeiling = nullptr;
    static SDL_Texture* paintpreviewTexture = NULL;
    static SDL_Texture* previewTexture = NULL;
    static SDL_Texture* previewTexture2 = NULL;
    static ImGuiTextFilter textureFilter;
    ImGui::Columns(2, "###mapedit");
    ImVec2 tileSize(20,20);
    for(auto& row : world){
        for(auto& tile : row){
            ImGui::PushID(tile.id);
            hTexture tile_handle = tile.m_pTexture->m_handle;

            hTexture color_handle = (tile.m_nType == Level::Tile_Wall) ? tile_handle : tile.m_hTextureFloor;
            std::string texturename = ITextureSystem->FilenameFromHandle(tile_handle);
             std::string clrtexturename = ITextureSystem->FilenameFromHandle(color_handle);
            auto text_info = texture_info.at(texturename);
            auto clrtext_info = texture_info.at(clrtexturename);

            ImVec4 color = clrtext_info.primary_color; //this is awesome
            if(selectedTile == &tile)
                color = ImVec4(color.x / 6.f, color.y /6.f, color.z / 6.f, 1.f); //better way
            ImGui::PushStyleColor(ImGuiCol_Button, color);
            if(ImGui::Button(std::to_string(tile.m_nType).c_str(), tileSize)){
                if(selectedTexture != nullptr)
                    m_texLastSelected = selectedTexture;
                selectedTile = &tile;
                selectedTexture = tile.m_pTexture;
                selectedTextureCeiling = tile.m_pTextureCeiling;
                selectedTextureFloor = tile.m_pTextureFloor;
                primary_texture_preview = texturename;
                previewTexture = text_info.texture_preview;
                type_preview = Editor::GetEnumName((Level::Tile_Type)tile.m_nType).data();
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
    }ImGui::SameLine();
    if(ImGui::Button("Unselect")){
        selectedTile = nullptr;
    }
    if(selectedTile == nullptr){
        ImGui::Text("No Tile Selected");
        ImGui::Columns();
        return;
    }
  
    ImGui::Text("Selected Tile: {%i, %i}", selectedTile->m_vecPosition.x, selectedTile->m_vecPosition.y);
  
    if(ImGui::BeginCombo("Type", type_preview, 0))
    {
        for(int i = 0; i < Level::Tile_Type_SIZE; ++i){
            const char* item_name = Editor::GetEnumName((Level::Tile_Type)i).data(); //magic_enum::enum_name((Level::Tile_Type)i).data();
            ImGui::PushID(&item_name);
            if(ImGui::Selectable(item_name, selectedTile->m_nType == i)){
                type_preview = item_name;
                selectedTile->m_nType = i;
            }
            ImGui::PopID();
        }

        ImGui::EndCombo();
    }

    if(selectedTile->m_nType == Level::Tile_Empty)
    {
        TexturePicker("Ceiling", selectedTextureCeiling, previewTexture, ceiling_texture_preview, &textureFilter);
        TexturePicker("Floor", selectedTextureFloor, previewTexture2, floor_texture_preview, &textureFilter);
        if(selectedTextureCeiling != nullptr && previewTexture != NULL)
        {
            auto strCeil = ITextureSystem->FilenameFromHandle(selectedTextureCeiling->m_handle);

            previewTexture = texture_info.at(strCeil).texture_preview;
            ImGui::Image(previewTexture, ImVec2(64, 64));
           // selectedTile->UpdateTexture(selectedTextureCeiling, TileTexture_Ceiling);
        }
        if(selectedTextureFloor != nullptr && previewTexture2 != NULL)
        {
            auto strFloor = ITextureSystem->FilenameFromHandle(selectedTextureFloor->m_handle);

            previewTexture2 = texture_info.at(strFloor).texture_preview;
            if(selectedTextureCeiling != nullptr && previewTexture != NULL)
                ImGui::SameLine();
            ImGui::Image(previewTexture2, ImVec2(64, 64));
          //  selectedTile->UpdateTexture(selectedTextureFloor, TileTexture_Floor); //not too happy about how ceil/flr ended up, shoulda just been 2 textures
        }
    }
    else
    {
        TexturePicker("Wall", selectedTexture, previewTexture, primary_texture_preview, &textureFilter);
        if(selectedTexture != nullptr && previewTexture != NULL)
        {
            ImGui::Image(previewTexture, ImVec2(64, 64));
          //  selectedTile->UpdateTexture(selectedTexture);
        }
    }
    textureFilter.Draw("Filter ##text");
    ImGui::SeparatorText("Tools");
    static bool all_floor = false;
    static bool all_ceiling = false;
    static bool all_wall = false;
    static bool all_default = false;
    ImGui::Checkbox("Floor##flrsetcb", &all_floor); ImGui::SameLine();
    ImGui::Checkbox("Ceiling##ceilsetcb", &all_ceiling); ImGui::SameLine();
    ImGui::Checkbox("Wall##wallsetcb", &all_wall); ImGui::SameLine();
    ImGui::Checkbox("All##allsetcb", &all_default);
    if(ImGui::Button("SetForAll") && selectedTexture && selectedTextureCeiling && selectedTextureFloor){
        for(auto& row : world){
            for(auto& tile : row){
                if(all_default){
                     tile.UpdateTexture(selectedTextureFloor, TileTexture_Floor);
                     tile.UpdateTexture(selectedTextureCeiling, TileTexture_Ceiling);
                     tile.UpdateTexture(selectedTexture); continue;
                }
                if(all_floor && tile.m_nType == Level::Tile_Empty){
                    tile.UpdateTexture(selectedTextureFloor, TileTexture_Floor);
                }
                if(all_ceiling && tile.m_nType == Level::Tile_Empty){
                    tile.UpdateTexture(selectedTextureCeiling, TileTexture_Ceiling);
                }
                if(all_wall && tile.m_nType == Level::Tile_Wall){
                    tile.UpdateTexture(selectedTexture);
                }
            }
        }

    }
    //do a different tab for paint mode
    TexturePicker("Paint", paintTexture, paintpreviewTexture, paint_texture_preview, &textureFilter);
    if(paintTexture != nullptr && paintpreviewTexture != NULL)
    {
            ImGui::Image(paintpreviewTexture, ImVec2(64, 64));   
              if(ImGui::IsKeyDown(ImGuiKey_Space) ){
                    selectedTile->m_nType = Level::Tile_Wall;
                    selectedTile->UpdateTexture(paintTexture);
                }     
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
    //add files to manifest etc
    ImGui::SeparatorText("Very unfinished memory leak galore");
    static std::string type_preview = "--";
    static bool view_textures = false;
    static bool view_other = false;
    if(ImGui::BeginCombo("Type", type_preview.c_str(), 0))
    {
        ImGui::PushID("materialrscoption");
        if(ImGui::Selectable("Materials", &view_textures)){
            view_textures = true;
            view_other = false;
            type_preview = "Materials";
        }
        ImGui::PopID();
        ImGui::PushID("leveldatarscoption");
        if(ImGui::Selectable("Level Data", &view_other)){
            view_textures = false;
            view_other = true;
            type_preview = "Level Data";
        }
        ImGui::PopID();
        ImGui::EndCombo();
    }

    if(view_textures)
        return drawMaterialView();
    
    ImGui::Text("Level Data View Goes Here");

}

void CEditor::drawMaterialView()
{
    static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");
    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
    
    static auto material_dir = IResourceSystem->GetDirectoryStructure("material");

    static std::pair<std::string, std::string>* selectedFile = nullptr;
    //load textures for preview... annoying as fuck
    //just load up the textures you actually want and add imgui text filter, its the natural textures that arent optimized
    //have one tab here for loading folders/files one tab for browsing
    //text filter the list to shit and make it render nice 
    /*
    ======
    [] [] [] []
    [] [] [] []
    ====
    with [] = thumbnail
    
    */
    ImGui::SameLine();
    if(ImGui::Button("Reload")){
        InitTextureInfo(); //MEMORY LEAK MEMORY LEAK
    }
    ImGui::SameLine();
    if(ImGui::Button("Save")){
        IResourceSystem->SaveTextureDefinition();
    }
    ImGui::Columns(2, "##matviewcols");
    static ImGuiTextFilter matFilter;
    matFilter.Draw("Filter Subdir: ##filter");

    if(ImGui::BeginListBox("/material/", ImVec2(250, 600)))
    {
        for(auto& pair : material_dir)
        {
            if(!matFilter.PassFilter(pair.second.c_str())) continue;
            ImGui::PushID(&pair);
            if(ImGui::Selectable(pair.first.c_str(), &pair == selectedFile)){
                selectedFile = &pair;
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
    ImGui::NextColumn();
    if(selectedFile == nullptr){
        ImGui::Text("select a file");
        ImGui::Columns(); return;

    }


    

    ImGui::Text(selectedFile->first.c_str());
    ImGui::Text(selectedFile->second.c_str());
    ImGui::SeparatorText("Add To Texture Definition");
    if(ImGui::Button("This")){
        std::string path = ITextureSystem->TextureNameToFile(selectedFile->first);
        if(!path.empty()){
            ITextureSystem->LoadTexture(selectedFile->first);
        }
    }
    ImGui::SameLine();
    if(ImGui::Button("Filtered")){
        for(auto& pair : material_dir)
        {
            if(!matFilter.PassFilter(pair.second.c_str())) continue;
            std::string path = ITextureSystem->TextureNameToFile(pair.first);
            if(!path.empty()){
                ITextureSystem->LoadTexture(pair.first);
            }
        }
    }  ImGui::SameLine();
    if(ImGui::Button("Subdir")){
        for(auto& pair : material_dir)
        {
            if(pair.second.compare(selectedFile->second)) continue;
            std::string path = ITextureSystem->TextureNameToFile(pair.first);
            if(!path.empty()){
                ITextureSystem->LoadTexture(pair.first);
            }
        }
    }

    ImGui::Columns();

}


void CEditor::TexturePicker(const char* title, texture_t*& selectedTexture, SDL_Texture*& previewTexture, std::string& preview, ImGuiTextFilter* filter)
{
    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");

    if(ImGui::BeginCombo(title, preview.c_str(), ImGuiComboFlags_HeightLarge))
    {
        for(const auto& entry : texture_info)
        {
            if(!filter->PassFilter(entry.first.c_str())) continue;
            ImGui::PushID(&entry);
            std::string name = entry.first;
            if(ImGui::Selectable(name.c_str(), selectedTexture == entry.second.texture))
            {
                selectedTexture = entry.second.texture;
                preview = ITextureSystem->FilenameFromHandle(selectedTexture->m_handle);
                previewTexture = entry.second.texture_preview;
               // m_texLastSelected = selectedTexture;
            }
            ImGui::SameLine();
            ImGui::Image(entry.second.texture_preview, ImVec2(32, 32));
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    
}