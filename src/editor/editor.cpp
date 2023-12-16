#include "editor.hpp"
#include <engine/engine.hpp>
#include <imgui_stdlib.h>
#include <imgui_impl_sdlrenderer3.h>
#include <imgui_impl_sdl3.h>
#include <SDL3/SDL.h>
#include "editor_helpers.hpp"
#include <renderer/render_helpers.hpp>
#include <util/misc.hpp>

#include <entity/dynamic/enemy/CEnemySoldier.hpp>
#include <data/CAnimData.hpp>
#define MENULOG(fmt, ...) engine->log(fmt, __VA_ARGS__)

void CEditor::drawSoundView()
{
    ImGui::SeparatorText("sound info");
                
    static float vol = 1.f;
    static float pan = 0.f;
    ImGui::SliderFloat("volume", &vol, 0.0f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("pan", &pan, -1.0f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    if (ImGui::Button("gunshot0"))
    {
        engine->SoundSystem()->PlaySound("dev_gunshot0", vol, pan);
    }ImGui::SameLine();
    if (ImGui::Button("mp5"))
    {
        engine->SoundSystem()->PlaySound("mp5", vol, pan);
    }ImGui::SameLine();
    if (ImGui::Button("hey"))
    {
            engine->SoundSystem()->PlaySound("soldier_hey", vol, pan);
    }ImGui::SameLine();
    if (ImGui::Button("music Ogg"))
    {
        engine->SoundSystem()->PlaySound("Cat", vol, pan);
    } 
    auto snd = engine->SoundSystem();
    ImGui::Text("StreamGroup");
    auto& streamgroup = snd->streams; 
    for (auto& stream : streamgroup.m_group)
    {
        if(stream.in_use && stream.src){
            ImGui::PushID(&stream);
            ImGui::Text("stream: dur: %li ms  playing %s @ %.2f vol %.2f pan", stream.duration, stream.src->m_name, stream.src->m_volume, 99.f);
            ImGui::PopID();
        }
    }

}

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

    if(!texture_info.empty()) // NO MORE GIANT MEMORY LEAK!!
    {
        for(auto& entry : texture_info){
            auto tex = entry.second;
            if(tex.texture_preview != NULL)
                SDL_DestroyTexture(tex.texture_preview);
        }
    }

    
     texture_info.clear(); 
    for (const auto &entry : ITextureSystem->texture_lookup)
    {
        auto texture = ITextureSystem->GetTextureData(entry.first);
        editor_texture_t to_add{
            .subpath = "",
            .texture_preview = SDL_CreateTextureFromSurface(m_renderer, texture->m_texture),
            .texture = texture,
            .primary_color = Editor::SDLClrToImClr4(Editor::GenerateAverageColor(texture)),
        };
        texture_info.emplace(entry.second, to_add);
    }
}

void CEditor::render(CRenderer *renderer)
{
    smith_renderer = renderer;
    static bool changedLastFrame = false;
    static auto IInputSystem = engine->CreateInterface<CInputSystem>("IInputSystem");
    IInputSystem->m_devMenuOpen = m_bIsOpen;
    if (IInputSystem->IsKeyDown(SDL_SCANCODE_BACKSLASH))
    {
        if (!changedLastFrame)
        {
            m_bIsOpen = !m_bIsOpen;
            changedLastFrame = true;
        }
    }
    else
        changedLastFrame = false;

    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
    static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    auto player = IEntitySystem->GetLocalPlayer();
    if (auto draw = ImGui::GetBackgroundDrawList(); draw) // just for scope really
    {
        static constexpr auto text_color = IM_COL32(255, 255, 255, 255);

        if (settings.show_pos)
        {
            auto p = player->GetPosition();
            std::string str_playerpos = Util::stringf("(%.1f, %.1f, %.1f)", p.x, p.y, p.z);
            auto textSize = ImGui::CalcTextSize(str_playerpos.c_str());
            const ImVec2 position(smith_renderer->GetFullWidth()  - 10, smith_renderer->GetFullHeight() - 10);
            draw->AddText(position - textSize, text_color, str_playerpos.c_str());
        }
        if (settings.show_cam)
        {
            auto c = player->Camera().m_vecPlane;
            auto d = player->Camera().m_vecDir;
            std::string str_camplane = Util::stringf("p(%.3f, %.3f) | d(%.3f, %.3f)", c.x, c.y, d.x, d.y);
            auto camtextSize = ImGui::CalcTextSize(str_camplane.c_str());
            const ImVec2 camposition(smith_renderer->GetFullWidth()  - 10, smith_renderer->GetFullHeight() - 25);
            draw->AddText(camposition - camtextSize, text_color, str_camplane.c_str());
        }
        if (settings.fps)
        {
            static float fps_min = 10000.f;
            static float fps_max = 0.f;
            float fps = IEngineTime->GetFPS();
            float fps_avg = IEngineTime->GetFPSAvg();
            if (fps < fps_min)
                fps_min = fps;
            if (fps > fps_max)
                fps_max = fps;
            if (fps_max > 10000)
                fps_max = 0;
            std::string fps_str = Util::stringf("%.2f | (%.1f, %.1f)", fps_avg, fps_min, fps_max);
            auto textSize_fps = ImGui::CalcTextSize(fps_str.c_str()) / 2; // textSize_fps.x =- 1.f;
            draw->AddText(textSize_fps, text_color, fps_str.c_str());
        }
        if (settings.ent_info)
        {
            auto cam = renderer->GetActiveCamera();
            for (auto ent : IEntitySystem->iterableList())
            {
                if (ent->IsLocalPlayer())
                    continue;

                auto screen = cam->WorldToScreen(ent->GetPosition());
                ImVec2 tp = {screen.x, screen.y};
                std::string ent_info = Util::stringf("%s / %i", ent->GetName().c_str(), ent->GetID());
                auto ent_ts = ImGui::CalcTextSize(ent_info.c_str());

                draw->AddText(tp - ent_ts, text_color, ent_info.c_str());
            }
        }
            std::string str_hp = Util::stringf("%d / %d | %d[%d]", player->GetHealth(), player->m_max_health, player->GetActiveWeapon()->GetCurrentAmmo(), player->GetActiveWeapon()->GetReserveAmmo());
            auto camtextSize = ImGui::CalcTextSize(str_hp.c_str());
            const ImVec2 camposition(camtextSize.x + 35, smith_renderer->GetFullHeight() - 25);
            draw->AddText(20.f, camposition , text_color, str_hp.c_str());

    }

    if (!isOpen())
        return;
    smith_renderer = renderer;
    SDL_SetRelativeMouseMode(SDL_FALSE);

    if (!m_bHasInit)
        Init();
    ImVec2 windowSize(UI_W, UI_H);
    ImVec2 windowPos(0, 0);
    static float nextAlpha = 0.2f;
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowBgAlpha(nextAlpha);
    static auto map_name = std::string("SmithEditor v0 | ").append(  ILevelSystem->m_Level->getName());
    
    ImGui::Begin(map_name.c_str());

    // the code here can be a bit greasy because it is a dev tool but try not to make it too singletrack
    //^^ this didnt age well 12.13.23
    if (ImGui::BeginTabBar("###mode"))
    {
        if (ImGui::BeginTabItem("Dev View"))
        {
            drawSystemView();
            
            if (ImGui::CollapsingHeader("Sound"))
            {
                drawSoundView();
            }

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Map Edit"))
        {
            drawMapView();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("New Editor"))
        {
            drawMapView();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Resource"))
        {
            drawResourceView();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Scene"))
        {
            drawEntityView();
            /*
               gonna need to serialize entities :(
                   - either add tojson to CBaseEntity or a getserializer func that returns a custom CBaseSerializable sorta thing
                   - or just fuckin save classtype and write it as bytes and rip backwards compatibility..
                   - i see why code generators and shit exist now for big engines, like marking members as @saved or @networked etc
                   - final option: CBaseSerializableField, template for types etc, might be fun, every field gets added to a CSerializeControllerComponent or soemthing
                       -tldr: entities havent been fleshed out yet and we can probably make this simple for static props at least (subclass!!)


               update: need to clean up the entity mess fr
            */
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Lighting"))
        {
            drawLightView();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("options"))
        {
            ImGui::SliderFloat("UI Alpha", &nextAlpha, 0.0f, 1.0f);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void CEditor::drawMapView()
{
    // imgui demo 2366
    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");

    static tile_t *selectedTile = nullptr;
    auto &level = ILevelSystem->m_Level;
    auto &world = level->world;

    static std::string primary_texture_preview = "--";
    static std::string ceiling_texture_preview = "--";
    static std::string floor_texture_preview = "--";
    static std::string paint_texture_preview = "--";
    static const char *type_preview = "--";
    static texture_t *paintTexture = nullptr;
    static texture_t *selectedTexture = nullptr;
    static texture_t *selectedTextureFloor = nullptr;
    static texture_t *selectedTextureCeiling = nullptr;
    static SDL_Texture *paintpreviewTexture = NULL;
    static SDL_Texture *previewTexture = NULL;
    static SDL_Texture *previewTexture2 = NULL;
    static SDL_Texture *previewTexture3 = NULL;
    static ImGuiTextFilter textureFilter;
    ImGui::Columns(2, "###mapedit");
    ImVec2 tileSize(32, 32);
    for (auto &row : world)
    {
        for (auto &tile : row)
        {
            ImGui::PushID(tile.id);
            hTexture tile_handle = tile.m_pTexture->m_handle;

            hTexture color_handle = (tile.m_nType != Level::Tile_Empty) ? tile_handle : tile.m_hTextureFloor;
            std::string texturename = ITextureSystem->FilenameFromHandle(tile_handle);
            std::string clrtexturename = ITextureSystem->FilenameFromHandle(color_handle);
            auto text_info = texture_info.at(texturename);
            auto clrtext_info = texture_info.at(clrtexturename);

            ImVec4 color = clrtext_info.primary_color; // this is awesome
            if (selectedTile == &tile)
                color = ImVec4(color.x / 6.f, color.y / 6.f, color.z / 6.f, 1.f); // better way
            ImGui::PushStyleColor(ImGuiCol_Button, color);
            if (ImGui::Button(std::to_string(tile.m_nType).c_str(), tileSize))
            {
                if (selectedTexture != nullptr)
                    m_texLastSelected = selectedTexture;
                selectedTile = &tile;
                selectedTexture = tile.m_pTexture;
                selectedTextureCeiling = tile.m_pTextureCeiling;
                selectedTextureFloor = tile.m_pTextureFloor;
                ceiling_texture_preview = ITextureSystem->FilenameFromHandle(tile.m_hTextureCeiling);
                floor_texture_preview = ITextureSystem->FilenameFromHandle(tile.m_hTextureFloor);
                auto ceil_info = texture_info.at(ceiling_texture_preview);
                auto floor_info = texture_info.at(floor_texture_preview);
                primary_texture_preview = texturename;

                previewTexture = text_info.texture_preview;
                previewTexture2 = floor_info.texture_preview;
                previewTexture3 = ceil_info.texture_preview;
                type_preview = Editor::GetEnumName((Level::Tile_Type)tile.m_nType).data();
            }
            if (!(tile.m_vecPosition.x == MAP_SIZE - 1))
            {
                ImGui::SameLine();
            }
            ImGui::PopStyleColor();
            ImGui::PopID();
        }
    }
    ImGui::NextColumn();
    if (ImGui::Button("Save"))
    {
        IResourceSystem->SaveLevel();
    }
    ImGui::SameLine();
    if (ImGui::Button("Unselect"))
    {
        selectedTile = nullptr;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset"))
    {
        for (auto &row : world)
        {
            for (auto &tile : row)
            {
                int x = tile.m_vecPosition.x, y = tile.m_vecPosition.y;
                if (x == 0 || y == 0 || x == MAP_SIZE - 1 || y == MAP_SIZE - 1)
                    continue;
                tile.m_nType = Level::Tile_Empty;
            }
        }
    }

    if (selectedTile == nullptr)
    {
        ImGui::Text("No Tile Selected");
        ImGui::Columns();
        return;
    }

    ImGui::Text("Selected Tile: {%i, %i}", selectedTile->m_vecPosition.x, selectedTile->m_vecPosition.y);

    if (ImGui::BeginCombo("Type", type_preview, 0))
    {
        for (int i = 0; i < Level::Tile_Type_SIZE; ++i)
        {
            const char *item_name = Editor::GetEnumName((Level::Tile_Type)i).data(); // magic_enum::enum_name((Level::Tile_Type)i).data();
            ImGui::PushID(&item_name);
            if (ImGui::Selectable(item_name, selectedTile->m_nType == i))
            {
                type_preview = item_name;
                selectedTile->m_nType = i;
            }
            ImGui::PopID();
        }

        ImGui::EndCombo();
    }

    if (selectedTile->m_nType != Level::Tile_Wall)
    {
        TexturePicker("Ceiling", selectedTile, selectedTextureCeiling, previewTexture3, ceiling_texture_preview, &textureFilter, TileTexture_Ceiling);
        TexturePicker("Floor", selectedTile, selectedTextureFloor, previewTexture2, floor_texture_preview, &textureFilter, TileTexture_Floor);
        if (selectedTextureCeiling != nullptr && previewTexture3 != NULL)
        {
            auto strCeil = ITextureSystem->FilenameFromHandle(selectedTextureCeiling->m_handle);

            previewTexture3 = texture_info.at(strCeil).texture_preview;
            ImGui::Image(previewTexture3, ImVec2(64, 64));
            // selectedTile->UpdateTexture(selectedTextureCeiling, TileTexture_Ceiling);
        }
        if (selectedTextureFloor != nullptr && previewTexture2 != NULL)
        {
            auto strFloor = ITextureSystem->FilenameFromHandle(selectedTextureFloor->m_handle);

            previewTexture2 = texture_info.at(strFloor).texture_preview;
            if (selectedTextureCeiling != nullptr && previewTexture != NULL)
                ImGui::SameLine();
            ImGui::Image(previewTexture2, ImVec2(64, 64));
            // selectedTile->UpdateTexture(selectedTextureFloor, TileTexture_Floor); //not too happy about how ceil/flr ended up, shoulda just been 2 textures
        }
    }
    if (selectedTile->m_nType != Level::Tile_Empty)
    {
        TexturePicker("Wall", selectedTile, selectedTexture, previewTexture, primary_texture_preview, &textureFilter, TileTexture_Primary);
        if (selectedTexture != nullptr && previewTexture != NULL)
        {
            ImGui::Image(previewTexture, ImVec2(64, 64));
        }
    }
    textureFilter.Draw("Filter ##text");
    ImGui::SeparatorText("Tools");
    static bool all_floor = false;
    static bool all_ceiling = false;
    static bool all_wall = false;
    static bool all_default = false;
    ImGui::Checkbox("Floor##flrsetcb", &all_floor);
    ImGui::SameLine();
    ImGui::Checkbox("Ceiling##ceilsetcb", &all_ceiling);
    ImGui::SameLine();
    ImGui::Checkbox("Wall##wallsetcb", &all_wall);
    ImGui::SameLine();
    ImGui::Checkbox("All##allsetcb", &all_default);
    if (ImGui::Button("SetForAll") && selectedTexture && selectedTextureCeiling && selectedTextureFloor)
    {
        for (auto &row : world)
        {
            for (auto &tile : row)
            {
                if (all_default)
                {
                    tile.UpdateTexture(selectedTextureFloor, TileTexture_Floor);
                    tile.UpdateTexture(selectedTextureCeiling, TileTexture_Ceiling);
                    tile.UpdateTexture(selectedTexture);
                    continue;
                }
                if (all_floor && tile.m_nType == Level::Tile_Empty)
                {
                    tile.UpdateTexture(selectedTextureFloor, TileTexture_Floor);
                }
                if (all_ceiling && tile.m_nType == Level::Tile_Empty)
                {
                    tile.UpdateTexture(selectedTextureCeiling, TileTexture_Ceiling);
                }
                if (all_wall && tile.m_nType == Level::Tile_Wall)
                {
                    tile.UpdateTexture(selectedTexture);
                }
            }
        }
    }
    // do a different tab for paint mode
    TexturePicker("Paint", selectedTile, paintTexture, paintpreviewTexture, paint_texture_preview, &textureFilter, -1);
    if (paintTexture != nullptr && paintpreviewTexture != NULL)
    {
        ImGui::Image(paintpreviewTexture, ImVec2(64, 64));
        if (ImGui::IsKeyDown(ImGuiKey_Space))
        {
            auto type = Level::Tile_Wall;
            auto tex = TileTexture_Primary;
            if (all_floor)
            {
                type = Level::Tile_Empty;
                tex = TileTexture_Floor;
            }
            if (all_ceiling)
            {
                type = Level::Tile_Empty;
                tex = TileTexture_Ceiling;
            }
            selectedTile->m_nType = type;
            selectedTile->UpdateTexture(paintTexture, tex);
        }
    }

    ImGui::SliderFloat("Ceiling Height", &selectedTile->m_flCeiling, -25.f, 25.f);
    ImGui::SliderFloat("Floor Height", &selectedTile->m_flFloor, -25.f, 25.f);

    ImGui::Text("flags: %li", selectedTile->m_nFlags);
    if(ImGui::Button("toggle nocol")){
        bool v = selectedTile->NoCollision();
        selectedTile->SetNoClip(!v);
    }
    ImGui::Columns();
    // Editor::IVec2Str(tile.m_vecPosition).c_str(),
}

void CEditor::drawNewMapEdit()
{
}

void CEditor::ShowEntityObject(CBaseEntity *entity, ImVec2 offset, ImDrawList *draw_list, tile_t* lastTile)
{
    // Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
    ImGui::PushID(entity);
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
    static auto IInputSystem = engine->CreateInterface<CInputSystem>("IInputSystem");
    // Text and Tree nodes are less high than framed widgets, using AlignTextToFramePadding() we add vertical spacing to make the tree lines equal high.
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    bool node_open = ImGui::TreeNode("Entity", "%s_%u", entity->GetName().c_str(), entity->GetID());
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%s", entity->GetSubclass().c_str());
    auto pos = entity->GetPosition();
    const float GRID_STEP = 32.f;

    static const auto csoldier= CEntitySystem::CreateType("CEnemySoldier");
    if (entity->IsRenderable() && !(entity->IsLocalPlayer()) && entity->GetID() > 0 && (entity->GetType() != csoldier))
    {
        auto rdr = (CBaseRenderable *)entity;
        auto name = ITextureSystem->FilenameFromHandle(rdr->GetTextureHandle());
        auto editor_text = texture_info.at(name);
        ImGui::Text("mat: %s", name.c_str());

        float offset_x = GRID_STEP * (float)pos.x + offset.x;
        float offset_y = GRID_STEP * (float)pos.y + offset.y;

        draw_list->AddImage(editor_text.texture_preview, ImVec2(pos.x + offset_x, pos.y + offset_y), ImVec2(pos.x + offset_x + GRID_STEP, pos.y + offset_y + GRID_STEP));
    }
    else if((entity->GetType() == csoldier)){
        float offset_x = GRID_STEP * (float)pos.x + offset.x;
        float offset_y = GRID_STEP * (float)pos.y + offset.y;

        ImU32 col = ( ((CEnemySoldier*)(entity))->GetHealth() > 0) ?  IM_COL32(255,0,0,200) : IM_COL32(0,255,0,120);        
        draw_list->AddCircleFilled(ImVec2(pos.x + offset_x, pos.y + offset_y), 10.f, col , 12);
    }
    if (node_open)
    {

        for (int i = 0; i < 1; i++)
        {
            ImGui::PushID(i); // Use field index as identifier.
            int components = 0;
            if (components)
            {
                // ShowPlaceholderObject("Child", 424242);
            }

            // Here we use a TreeNode to highlight on hover (we could use e.g. Selectable as well)
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;

            

            ImGui::TreeNodeEx("Properties", flags, "%d", i);

            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);

            ImGui::Text("Position {%.2f, %.2f, %.1f}", pos.x, pos.y, pos.z);

            //  ImGui::InputFloat("##value", &placeholder_members[i], 1.0f);
            //  else
            //  ImGui::DragFloat("##value", &placeholder_members[i], 0.01f);

            if (entity->IsRenderable() && !(entity->IsLocalPlayer()) && (entity->GetType() != csoldier))
            {
                auto rdr = (CBaseRenderable *)entity;
                auto name = ITextureSystem->FilenameFromHandle(rdr->GetTextureHandle());
                auto editor_text = texture_info.at(name);
                ImGui::Text("mat: %s", name.c_str());
            }
            if (entity->GetType() == csoldier)
            {
                auto enemy = (CEnemySoldier *)entity;
                ImGui::Text("Health {%i / %i}", enemy->GetHealth(), enemy->GetMaxHealth());

                ImGui::Text("behaviour %s", magic_enum::enum_name((CEnemySoldier::SoldierBehaviour)enemy->m_behaviour).data());
                auto path = enemy->GetPathFinder();
                if(ImGui::Button("set to last selected tile")){
                    if(lastTile != nullptr){
                        Vector2 set = lastTile->m_vecPosition;
                        set = { set.x + 0.3, set.y + 0.4};
                        enemy->SetPosition(set);
                        engine->info("set %s to {%.1f %.1f}", enemy->GetName().c_str(), set.x, set.y);
                    }
                }
                if (path->HasPath())
                {
                    ImGui::Text("%i / %li Steps", path->m_iPathIndex, path->m_iPathSize);
                    // engine->log("%i", path->path.size());
                    auto idx = path->m_iPathIndex;
                    int i = 0;
                    for (auto &node : path->path)
                    {

                        const float GRID_STEP = 32.f;
                        float offset_x = GRID_STEP * (float)(node.x + 0.5) + offset.x;
                        float offset_y = GRID_STEP * (float)(node.y + 0.5) + offset.y;

                        auto col = IM_COL32(0, 240, 255, 165);
                        if (i > idx)
                            col = IM_COL32(255, 100, 0, 165);
                        draw_list->AddCircleFilled(ImVec2(node.x + offset_x, node.y + offset_y), 5.f, col, 12);
                        i++;
                    }
                }
                else{
                    ImGui::Text("no path");
                }
            }
            else if (entity->IsLocalPlayer())
            {
                auto player = (CPlayer *)entity;
                float f = player->m_move.m_flForwardSpeed;
                ImGui::SliderFloat("ForwardSpeed", &f, 0.0f, 5.f, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp);
                player->m_move.m_flForwardSpeed = f;

                float s = player->m_move.m_flStrafeSpeed;
                ImGui::SliderFloat("StrafeSpeed", &s, 0.0f, 5.f, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp);
                player->m_move.m_flStrafeSpeed = s;

                float m = player->m_move.m_flSpeedModifier;
                ImGui::SliderFloat("SpeedModifier", &m, 0.0f, 5.f, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp);
                player->m_move.m_flSpeedModifier = m;

                ImGui::Text("inputsystem");
                float accel = IInputSystem->m_flMouseAccel;
                ImGui::SliderFloat("Accel", &accel, 0.0f, 15.f, "%.6f", ImGuiSliderFlags_Logarithmic);
                IInputSystem->m_flMouseAccel = accel;
                float scale = IInputSystem->m_flMouseScale;
                ImGui::SliderFloat("Mouse Scale", &scale, 0.0f, 0.5f, "%.6f", ImGuiSliderFlags_Logarithmic);
                IInputSystem->m_flMouseScale = scale;

                float sens = IInputSystem->m_flSensitivity;
                ImGui::SliderFloat("Mouse Sens", &sens, 0.0f, 10.f, "%.6f", ImGuiSliderFlags_Logarithmic);
                IInputSystem->m_flSensitivity = sens;
                if (ImGui::Button("Set spawn to cur pos"))
                {
                    ILevelSystem->m_Level->m_vecPlayerStart = Vector2(player->GetPosition());
                }
            }

            ImGui::NextColumn();

            ImGui::PopID();
        }
        ImGui::TreePop();
    }
    if (entity->IsLocalPlayer())
    {

        float offset_x = GRID_STEP * (float)(pos.x + 0.5) + offset.x;
        float offset_y = GRID_STEP * (float)(pos.y + 0.5) + offset.y;

        auto col = IM_COL32(255, 255, 255, 175);
        auto col2 = IM_COL32(0, 0, 0, 255);
        draw_list->AddCircleFilled(ImVec2(pos.x + offset_x, pos.y + offset_y), 8.f, col, 12);

        auto player = (CPlayer *)entity;

        auto look = (Vector2)pos + (player->Camera().m_vecDir.Normalize() * 15.f);

        draw_list->AddLine(ImVec2(pos.x + offset_x, pos.y + offset_y), ImVec2(look.x + offset_x, look.y + offset_y), col2, 2.f);
    }
    ImGui::PopID();
}

void CEditor::drawEntityView()
{

    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    // there is an example for this in the imguidemo
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
    static ImVec2 scrolling(0.0f, 0.0f);
    static bool opt_enable_grid = true;
    static bool opt_enable_context_menu = true;
    static bool adding_line = false;

    // ImGui::Checkbox("Enable context menu", &opt_enable_context_menu);
    // ImGui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));     // Disable padding
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255)); // Set a background color
    ImGui::BeginChild("canvas", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoMove);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();    // ImDrawList API uses screen coordinates!
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail(); // Resize canvas to what's available
    if (canvas_sz.x < 50.0f)
        canvas_sz.x = 50.0f;
    if (canvas_sz.y < 50.0f)
        canvas_sz.y = 50.0f;

    // canvas_sz = {0.f,0.f};
    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

    // Draw border and background color
    ImGuiIO &io = ImGui::GetIO();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

    const bool is_hovered = ImGui::IsItemHovered();                            // Hovered
    const bool is_active = ImGui::IsItemActive();                              // Held
    const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
    const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

    // Pan (we use a zero mouse threshold when there's no context menu)
    // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
    const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
    if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
    {
        scrolling.x += io.MouseDelta.x;
        scrolling.y += io.MouseDelta.y;
    }

    // Context menu (under default mouse threshold)
    ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    if (opt_enable_context_menu && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
        ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
    if (ImGui::BeginPopup("context"))
    {

        if (ImGui::MenuItem("todo", NULL, false, true))
        {
        }
        if (ImGui::MenuItem("todo", NULL, false, true))
        {
        }
        ImGui::EndPopup();
    }

    // Draw grid + all lines in the canvas
    // draw_list->PushClipRect(canvas_p0, canvas_p1, true);

    const float GRID_STEP = 32.0f;
    auto &level = ILevelSystem->m_Level;
    auto &world = level->world;

    for (float x = 0.f; x < 64.f; x += 1.0f)
    {
        draw_list->AddLine(ImVec2(x * GRID_STEP + canvas_p0.x, canvas_p0.y), ImVec2(x * GRID_STEP + canvas_p0.x, GRID_STEP * 64 + canvas_p0.y), IM_COL32(200, 200, 200, 40));
    }
    for (float y = 0.f; y < 64.f; y += 1.0f)
    {
        draw_list->AddLine(ImVec2(canvas_p0.x, y * GRID_STEP + canvas_p0.y), ImVec2(GRID_STEP * 64 + canvas_p0.x, y * GRID_STEP + canvas_p0.y), IM_COL32(200, 200, 200, 40));
    }
    static tile_t* lastTile = nullptr;
    for (auto &row : world)
    {

        for (auto &tile : row)
        {

            auto hTexture = tile.m_hTexture;
            if (tile.m_nType == Level::Tile_Empty)
                hTexture = tile.m_hTextureFloor;
            auto name = ITextureSystem->FilenameFromHandle(hTexture);
            auto editor_text = texture_info.at(name);
            auto pos = tile.m_vecPosition;

            float offset_x = GRID_STEP * (float)pos.x + canvas_p0.x;
            float offset_y = GRID_STEP * (float)pos.y + canvas_p0.y;
            ImGui::PushID(&tile);
            ImGui::SetCursorPos(ImVec2(pos.x + GRID_STEP * pos.x, pos.y + GRID_STEP * pos.y));

            draw_list->AddImage(editor_text.texture_preview, ImVec2(pos.x + offset_x, pos.y + offset_y), ImVec2(pos.x + offset_x + GRID_STEP, pos.y + offset_y + GRID_STEP));
            if (ImGui::InvisibleButton("##tile", {GRID_STEP, GRID_STEP}))
            { // ImGui::ImageButton
                engine->log("%s", name.c_str());
                lastTile = &tile;
            }
            ImGui::PopID();
        }
    }

    static bool pOpen = false;

    ImGui::EndChild();
    ImGui::SetNextWindowSize(ImVec2(UI_W / 3.5, UI_H - 150), ImGuiCond_FirstUseEver);
    static bool once = false;
    if (!once)
    {
        ImGui::SetNextWindowPos(ImVec2(smith_renderer->GetFullWidth() * 0.7, smith_renderer->GetFullHeight() * 0.05));
        once = true;
    }

    if (!ImGui::Begin("Example: Property editor", &pOpen))
    {
        ImGui::End();
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    if (ImGui::BeginTable("##split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Entity");
        ImGui::TableSetupColumn("Properties");
        ImGui::TableHeadersRow();

        // Iterate placeholder objects (all the same data)
        for (auto &ent : IEntitySystem->iterableList())
            ShowEntityObject(ent, ImVec2{canvas_p0.x, canvas_p0.y}, draw_list, lastTile);

        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
    ImGui::End();
}

void CEditor::drawResourceView()
{
    // add files to manifest etc
    ImGui::SeparatorText("Very unfinished memory leak galore");
    static std::string type_preview = "--";
    static bool view_textures = false;
    static bool view_other = false;
    static bool view_mateditor = false;
    static int view_idx = 0;

    if (ImGui::BeginCombo("Type", type_preview.c_str(), 0))
    {
        ImGui::PushID("materialrscoption");
        if (ImGui::Selectable("Material Import", view_idx == 1))
        {
            view_idx = 1;
            type_preview = "Material Import";
        }
        ImGui::PopID();
        ImGui::PushID("unusedtab");
        if (ImGui::Selectable("--------", view_idx == 2))
        {
            view_idx = 2;
            type_preview = "MOVED";
        }
        ImGui::PopID();
        ImGui::PushID("materialeditor");
        if (ImGui::Selectable("Material Editor", view_idx == 3))
        {
            view_idx = 3;
            type_preview = "Material Editor";
        }
        ImGui::PopID();
        ImGui::PushID("animeditor");
        if (ImGui::Selectable("Animation Editor", view_idx == 4))
        {
            view_idx = 4;
            type_preview = "Anim. Editor";
        }
        ImGui::PopID();
        ImGui::EndCombo();
    }

    if (view_idx == 1)
        return drawMaterialView();
    if (view_idx == 2)
    {
        ImGui::SeparatorText("moved!"); return;
    }
    if (view_idx == 3)
        return drawMaterialEditor();
    

    if (view_idx == 4)
    {
        return drawAnimView();
    }
    ImGui::Text("Level Data View Goes Here");
}

void CEditor::drawMaterialView()
{
    static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");
    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");

    static auto material_dir = IResourceSystem->GetDirectoryStructure("material");

    static std::pair<std::string, std::string> *selectedFile = nullptr;
    // load textures for preview... annoying as fuck
    // just load up the textures you actually want and add imgui text filter, its the natural textures that arent optimized
    // have one tab here for loading folders/files one tab for browsing
    // text filter the list to shit and make it render nice
    /*
    ======
    [] [] [] []
    [] [] [] []
    ====
    with [] = thumbnail

    */
    ImGui::SameLine();
    if (ImGui::Button("Reload"))
    {
        InitTextureInfo(); // MEMORY LEAK MEMORY LEAK
    }
    ImGui::SameLine();
    if (ImGui::Button("Save"))
    {
        IResourceSystem->SaveTextureDefinition();
    }
    ImGui::Columns(2, "##matviewcols");
    static ImGuiTextFilter matFilter;
    matFilter.Draw("Filter Subdir: ##filter");

    if (ImGui::BeginListBox("/material/", ImVec2(250, 600)))
    {
        for (auto &pair : material_dir)
        {
            if (!matFilter.PassFilter(pair.second.c_str()))
                continue;
            ImGui::PushID(&pair);
            if (ImGui::Selectable(pair.first.c_str(), &pair == selectedFile))
            {
                selectedFile = &pair;
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
    ImGui::NextColumn();
    if (selectedFile == nullptr)
    {
        ImGui::Text("select a file");
        ImGui::Columns();
        return;
    }

    ImGui::Text(selectedFile->first.c_str());
    ImGui::Text(selectedFile->second.c_str());
    ImGui::SeparatorText("Add To Texture Definition");
    if (ImGui::Button("This"))
    {
        std::string path = ITextureSystem->TextureNameToFile(selectedFile->first);
        if (!path.empty())
        {
            ITextureSystem->LoadTexture(selectedFile->first);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Filtered"))
    {
        for (auto &pair : material_dir)
        {
            if (!matFilter.PassFilter(pair.second.c_str()))
                continue;
            std::string path = ITextureSystem->TextureNameToFile(pair.first);
            if (!path.empty())
            {
                ITextureSystem->LoadTexture(pair.first);
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Subdir"))
    {
        for (auto &pair : material_dir)
        {
            if (pair.second.compare(selectedFile->second))
                continue;
            std::string path = ITextureSystem->TextureNameToFile(pair.first);
            if (!path.empty())
            {
                ITextureSystem->LoadTexture(pair.first);
            }
        }
    }

    ImGui::Columns();
}

void CEditor::drawMaterialEditor()
{
    static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");
    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    ImGui::SeparatorText("Material Editor");

    static ImGuiTextFilter filter;
    static texture_t *selectedTexture = nullptr;
    static SDL_Texture *previewTexture = NULL;
    static std::string tex_name;
    static std::string tex_filename;
    static editor_texture_t *tex_info = nullptr;
    ImGui::Columns(2, "##mateditor");
    static const auto pickerSize = ImVec2(UI_W / 2.5, UI_H * 0.85);
    filter.Draw("Filter (inc,-exc)", pickerSize.x - 10);
    if (ImGui::BeginListBox("Select Material", pickerSize))
    {
        for (const auto &entry : texture_info)
        {
            if (!filter.PassFilter(entry.first.c_str()))
                continue;
            ImGui::PushID(&entry);
            std::string name = entry.first;
            if (ImGui::Selectable(name.c_str(), selectedTexture == entry.second.texture))
            {
                selectedTexture = entry.second.texture;
                tex_name = ITextureSystem->FilenameFromHandle(selectedTexture->m_handle);
                previewTexture = entry.second.texture_preview;
                tex_info = &(texture_info.at(tex_name));
                const static auto subdir = IResourceSystem->GetResourceSubDir("material");
                tex_filename = IResourceSystem->FindResource(subdir, tex_name);
            }
            ImGui::SameLine();
            ImGui::Image(entry.second.texture_preview, ImVec2(32, 32));
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }

    ImGui::NextColumn();
    if (selectedTexture == nullptr || tex_info == nullptr)
    {
        ImGui::Columns();
        return;
    }
    ImGui::Text("%s / %x", tex_name.c_str(), selectedTexture->m_handle);
    ImGui::SameLine();
    if (ImGui::Button("Save"))
    {
        IResourceSystem->SaveTextureDefinition();
    }

    // SelectedTexture
    ImGui::Image(previewTexture, ImVec2(256, 256));
    ImGui::SameLine();
    ImGui::Text("%s / %x", tex_name.c_str(), selectedTexture->m_handle);
    ImGui::TextWrapped("%s", tex_filename.c_str());
    ImGui::Text("Size -> { %ix%i }", selectedTexture->m_size.x, selectedTexture->m_size.y);
    auto clr = Render::TextureToSDLColor(selectedTexture->m_clrKey);
    ImGui::Text("Mask Color {%i %i %i %i}", clr.r, clr.b, clr.g, clr.a);

    static int v[4];
    if (ImGui::CollapsingHeader("Edit ##mask"))
    {
        ImVec4 imclr = ImGui::ColorConvertU32ToFloat4(IM_COL32((uint8_t)v[0], (uint8_t)v[1], (uint8_t)v[2], (uint8_t)v[3]));
        ImGui::ColorButton("##mask", imclr, ImGuiColorEditFlags_NoPicker, {32, 32});

        ImGui::InputInt4("Mask Color", v);
        ImGui::SameLine();

        if (ImGui::SmallButton("Apply"))
        {
            clr = {(uint8_t)v[0], (uint8_t)v[1], (uint8_t)v[2], (uint8_t)v[3]};
            selectedTexture->m_clrKey = Render::SDLColorToWorldColor(clr);
        }
    }

    ImGui::Columns();
}

void CEditor::drawLightView()
{
    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    // there is an example for this in the imguidemo
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
    static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");

    static ImVec2 scrolling(0.0f, 0.0f);

    static bool runMapView = true;
    ImGui::Checkbox("show mapview", &runMapView);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));     // Disable padding
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255)); // Set a background color
    ImGui::SetNextWindowBgAlpha(0.1f);
    ImGui::BeginChild("canvaslight", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoMove);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();    // ImDrawList API uses screen coordinates!
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail(); // Resize canvas to what's available
    if (canvas_sz.x < 50.0f)
        canvas_sz.x = 50.0f;
    if (canvas_sz.y < 50.0f)
        canvas_sz.y = 50.0f;

    // canvas_sz = {0.f,0.f};

    const double x_mod = 0.5, y_mod = 0.7;
    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + (canvas_sz.x * x_mod), canvas_p0.y + (canvas_sz.y * y_mod));

    // Draw border and background color
    ImGuiIO &io = ImGui::GetIO();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    const bool is_hovered = ImGui::IsItemHovered();                            // Hovered
    const bool is_active = ImGui::IsItemActive();                              // Held
    const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
    const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

    const float GRID_STEP = 32.0f;
    auto &level = ILevelSystem->m_Level;
    auto &world = level->world;

    static tile_t *selectedTile = nullptr;
    static bool visualizeLighting = true;
    static float visualizeAlphaMod = 1.f;
    if (runMapView)
    {
        for (auto &row : world)
        {
            for (auto &tile : row)
            {

                auto hTexture = tile.m_hTexture;
                if (tile.m_nType == Level::Tile_Empty)
                    hTexture = tile.m_hTextureFloor;
                auto name = ITextureSystem->FilenameFromHandle(hTexture);
                auto editor_text = texture_info.at(name);
                auto pos = tile.m_vecPosition;

                float offset_x = GRID_STEP * (float)pos.x + canvas_p0.x;
                float offset_y = GRID_STEP * (float)pos.y + canvas_p0.y;
                ImGui::PushID(&tile);
                ImGui::SetCursorPos(ImVec2(pos.x + GRID_STEP * pos.x, pos.y + GRID_STEP * pos.y));
                Color vxvtr_clr = tile.getVoxelAt(1, 1, 1)->m_light;

                // if you wanna really do this right then make a texture the size of the grid, and draw a mini lightmap on it.. or do 9 mini images with uv set and tint of avg column color

                draw_list->AddImage(editor_text.texture_preview, ImVec2(pos.x + offset_x, pos.y + offset_y), ImVec2(pos.x + offset_x + GRID_STEP, pos.y + offset_y + GRID_STEP));
                if (visualizeLighting)
                    draw_list->AddImage(editor_text.texture_preview, ImVec2(pos.x + offset_x, pos.y + offset_y), ImVec2(pos.x + offset_x + GRID_STEP, pos.y + offset_y + GRID_STEP),
                                        ImVec2(0, 0), ImVec2(1, 1), IM_COL32(vxvtr_clr.r(), vxvtr_clr.g(), vxvtr_clr.b(), vxvtr_clr.a() * visualizeAlphaMod));

                if (ImGui::InvisibleButton("##tile", {GRID_STEP, GRID_STEP}))
                { // ImGui::ImageButton
                    engine->log("%s", name.c_str());
                    selectedTile = &tile;
                    for (int dir = NORTH; dir <= WEST; ++dir)
                    {
                        auto nbr = ILevelSystem->GetTileNeighbor(&tile, dir);
                        const char *dn = magic_enum::enum_name((Cardinal_Directions)dir).data();
                        if (!nbr)
                        {
                            engine->log("{%i %i} no neighbor %s", pos.x, pos.y, dn);
                            continue;
                        }

                        engine->log(" {%i %i} %s", nbr->m_vecPosition.x, nbr->m_vecPosition.y, (dn) ? dn : "lol");
                    }

                    for (int x = 0; x < TILE_SECTORS; ++x)
                        for (int y = 0; y < TILE_SECTORS; ++y)
                            for (int z = 0; z < TILE_SECTORS; ++z)
                            {
                                // Vector p = tile.getSectorCenterRelativeCoords(x, y, z) + Vector(tile.m_vecPosition.x, tile.m_vecPosition.y, 0.f);
                                // engine->log(" {%.3f %.3f %.3f } %i %i ", p.x, p.y, p.z, tile.m_vecPosition.x, tile.m_vecPosition.y );
                            }
                }
                ImGui::PopID();
            }
        }
    }

    static CLight *selectedLight = nullptr;

    static bool drawRays = true;
    static bool drawHitsOnly = false;
    static bool drawMissesOnly = false;
    static bool drawRayGoals = true;
    static float minRayLength = 0.f;
    static float maxRayLength = 50.f;
    static int ray_filter = 50;
    static ImGuiTextFilter light_filter("");

    if (ray_filter < 1)
        ray_filter = 1;
    constexpr auto rayHitClr = IM_COL32(15, 245, 30, 100);
    constexpr auto rayMissClr = IM_COL32(245, 0, 30, 50);
    constexpr auto rayGoalClr = IM_COL32(245, 245, 245, 50);
    for (auto &entry : ILightingSystem->light_list)
    {
        if (!runMapView)
            continue; // bad
        auto light = entry.second;
        Vector2 pos = light->GetPosition();
        float offset_x = GRID_STEP * (float)pos.x + canvas_p0.x;
        float offset_y = GRID_STEP * (float)pos.y + canvas_p0.y;
        auto ui_pos = ImVec2(pos.x + offset_x, pos.y + offset_y);
        int i = 0;
        if (selectedLight != nullptr && light != selectedLight)
            continue;
        std::string last{light->GetName().back()};
        if (!light_filter.PassFilter(last.c_str()))
            continue; // this will break soon
        for (auto &ray : light->rays)
        {
            if (!drawRays)
                continue;
            // v2 ray end  v2 ray goal b ray hit
            bool ray_hit = std::get<2>(ray);
            if (!ray_hit && drawHitsOnly)
                continue; // so fals means it actually hit bc i am so good at naming
            i++;
            if (i % ray_filter != 0)
                continue;
            auto &ray_end = std::get<0>(ray);
            auto &ray_aim = std::get<1>(ray);
            if (Vector2(ray_end - pos).Length() < minRayLength)
                continue;
            if (Vector2(ray_end - pos).Length() > maxRayLength)
                continue;
            float offset_endx = GRID_STEP * (float)ray_end.x + canvas_p0.x;
            float offset_endy = GRID_STEP * (float)ray_end.y + canvas_p0.y;
            if (drawMissesOnly && ray_hit)
                continue; // MAY I MENTION AGAIN A HIT IS ACTUALLY WHEN WE DIDNT RECORD A HIT G
            // GOD DAMN IT

            draw_list->AddLine(ui_pos, ImVec2(ray_end.x + offset_endx, ray_end.y + offset_endy), (ray_hit) ? rayHitClr : rayMissClr);
            if (drawRayGoals && !ray_hit)
            {
                float offset_aimx = GRID_STEP * (float)ray_aim.x + canvas_p0.x;
                float offset_aimy = GRID_STEP * (float)ray_aim.y + canvas_p0.y;
                draw_list->AddLine(ImVec2(ray_end.x + offset_endx, ray_end.y + offset_endy), ImVec2(ray_aim.x + offset_aimx, ray_aim.y + offset_aimy), rayGoalClr);
            }
            // draw_list->AddLine(ui_pos, ImVec2(ray_aim.x + offset_x, ray_aim.y + offset_y), rayMissClr);
        }
        if (selectedLight != nullptr && light == selectedLight)
        {
            draw_list->AddCircle(ui_pos, 3.f, Editor::ColorToIU32(light->GetColor()), 8, 3.f);
        }
        draw_list->AddCircle(ui_pos, 18.f, Editor::ColorToIU32(light->GetColor()), 8, 3.f);
        draw_list->AddCircle(ui_pos, light->GetRange() * GRID_STEP, Editor::ColorToIU32(light->GetColor(), true), 32, 0.5f);
    }
    static bool drawPoints = false;
    if (drawPoints)
    {
        for (auto &pt : ILightingSystem->tested_points)
        {
            auto pos = pt;
            float offset_x = GRID_STEP * (float)pos.x + canvas_p0.x;
            float offset_y = GRID_STEP * (float)pos.y + canvas_p0.y;
            auto ui_pos = ImVec2(pos.x + offset_x, pos.y + offset_y);
            draw_list->AddCircle(ui_pos, 3.f, Editor::ColorToIU32(Color::Olive()), 6, 1.f);
        }
    }

    static bool pOpen = false;

    ImGui::EndChild();
    ImGui::SetNextWindowSize(ImVec2(UI_W / 3.5, UI_H - 150), ImGuiCond_FirstUseEver);
    static bool once = false;
    if (!once)
    {
        ImGui::SetNextWindowPos(ImVec2(smith_renderer->GetFullWidth() * 0.7, smith_renderer->GetFullHeight() * 0.05));
        once = true;
    }

    if (!ImGui::Begin("Lighting Editor", &pOpen))
    {
        ImGui::End();
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    if (ImGui::Button("Regenerate Lighting"))
    {
        ILightingSystem->RegenerateLighting();
    }
    if (ImGui::BeginTabBar("###lightmodes"))
    {
        if (ImGui::BeginTabItem("Params and properties"))
        {
            light_params *p = &(ILightingSystem->params);
            ImGui::Text("Light Params");

            ImGui::SliderFloat("A", &p->a, -0.2f, 5.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
            ImGui::SliderFloat("B", &p->b, -0.2f, 5.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
            ImGui::SliderFloat("Min Intensity", &p->minIntensity, 0.0f, 1.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
            ImGui::SliderFloat("Alpha Mod", &p->alphaMod, 0.0f, 2.0f, "%.4f");
            ImGui::SliderFloat("Color Mod", &p->colorModifier, 0.0f, 2.0f, "%.4f");
            ImGui::SliderFloat("Final Mod", &p->finalModifier, 0.0f, 2.0f, "%.4f");
            ImGui::SliderFloat("Rolloff Mod", &p->finalModifier, 0.0f, 2.0f, "%.4f");
            ImGui::SliderFloat("Intensity Mod", &p->finalModifier, 0.0f, 2.0f, "%.4f");
            ImGui::SliderFloat("InterpFrac", &p->interpFraction, 0.0f, 1.f, "%.4f");
            ImGui::Checkbox("Neighbor Blend", &p->neighbor_interp);
            ImGui::Checkbox("Dynamic", &p->dynamic);
            if (ImGui::Button("Toggle Debug"))
            {
                ILightingSystem->Debug(!ILightingSystem->Debug());
            }

            ImGui::InputInt("Method", &p->method, 1);
            // if(p->mergeMethod > 1) p ->mergeMethod = 1;
            if (p->method < 0)
                p->method = 0;
            ImGui::SeparatorText("tile info");

            if (selectedTile != nullptr)
            {
                auto &pos = selectedTile->m_vecPosition;
                const char *type_name = Editor::GetEnumName((Level::Tile_Type)selectedTile->m_nType).data();
                ImGui::Text("Tile @ {%i %i}, %s", pos.x, pos.y, type_name);
                for (int x = 0; x < TILE_SECTORS; ++x)
                    for (int y = 0; y < TILE_SECTORS; ++y)
                        for (int z = 0; z < TILE_SECTORS; ++z)
                        {

                            auto voxel = selectedTile->getVoxelAt(x, y, z);
                            ImGui::PushID(&voxel);
                            Editor::ColorPreview(voxel->m_light);
                            ImGui::SameLine();
                            ImGui::Text("{%i %i %i} | clr %s | [%i/6]", x, y, z, voxel->m_light.s().c_str(), voxel->m_neighborsize);
                            static bool showNeigh = false;
                            ImGui::Checkbox("neighbors", &showNeigh);
                            if (showNeigh)
                            {
                                for (int i = 0; i < voxel->m_neighborsize; ++i)
                                {
                                    if (voxel->m_neighbors[i] != Color::None())
                                    {
                                        Editor::ColorPreview(voxel->m_neighbors[i]);
                                        ImGui::SameLine();
                                        ImGui::Text("nbr: %i / %s", i, voxel->m_neighbors[i].s().c_str());
                                    }
                                }
                            }
                            ImGui::PopID();
                        }
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Lights"))
        {
            if (ImGui::Button("Add Light"))
            {
                auto light = ILightingSystem->AddLightByClassname("CLightOverhead");
                light->SetPosition({13.f, 13.f, 2.f});
                light->SetColor(Color::FluorescentLight());
                light->SetRange(3.f);
            }
            ImGui::SameLine();
            if (ImGui::Button("clr"))
            {
                selectedLight = nullptr;
            }
            static float clr[4];
            for (auto &entry : ILightingSystem->light_list)
            {
                auto light = entry.second;
                ImGui::PushID(light);
                auto p = light->GetPosition();
                ImGui::Text("%s {%.1f %.1f %.1f } | %s", light->GetName().c_str(), p.x, p.y, p.z, light->GetColor().s().c_str());
                ImGui::SameLine();
                if (ImGui::SmallButton("[*]"))
                {
                    selectedLight = light;
                }
                if (selectedLight != nullptr && light == selectedLight)
                {
                    ImGui::SliderFloat("Brightness", &light->m_flBrightness, 0.0f, 1.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
                    ImGui::SliderFloat("Intensity", &light->m_flIntensity, 0.0f, 1.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
                    ImGui::SliderFloat("Range", &light->m_flRange, 0.0f, 50.0f, "%.4f");
                    ImGui::SliderFloat("X", &light->m_vecPosition.x, 0.0f, 50.0f, "%.4f");
                    ImGui::SliderFloat("Y", &light->m_vecPosition.y, 0.0f, 50.0f, "%.4f");
                    Color s = Editor::colorPicker("Color", light->GetColor(), ImGuiColorEditFlags_DisplayRGB);
                    light->m_color = s;

                    if (ImGui::Button("warp here"))
                    {
                        IEntitySystem->GetLocalPlayer()->SetPosition(light->m_vecPosition.x, light->m_vecPosition.y, 0.f);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("set to pos"))
                    {
                        light->SetPosition(IEntitySystem->GetLocalPlayer()->GetPosition());
                        light->m_vecPosition.z = 1.0f; // Z is so messed up
                    }
                }

                ImGui::PopID();
                ImGui::Separator();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Map"))
        {
            light_filter.Draw("filter", UI_W / 4.5);
            ImGui::Checkbox("Draw Points", &drawPoints);
            ImGui::Checkbox("Visualize Light", &visualizeLighting);
            if (visualizeLighting)
                ImGui::InputFloat("alphamod", &visualizeAlphaMod, 0.05f, 0.2f);
            visualizeAlphaMod = std::clamp(visualizeAlphaMod, 0.f, 1.5f);
            ImGui::SeparatorText("Rays");
            ImGui::Checkbox("Draw Rays", &drawRays);
            ImGui::Checkbox("Hits only", &drawHitsOnly);
            ImGui::Checkbox("Misses only", &drawMissesOnly);
            ImGui::Checkbox("Draw Goals", &drawRayGoals);
            ImGui::SliderFloat("Min. Dist", &minRayLength, 0.0f, 40.0f, "%.2f");
            ImGui::SliderFloat("Max. Dist", &maxRayLength, 0.0f, 40.0f, "%.2f");
            ImGui::InputInt("Modulo filter", &ray_filter, 1);
            ImGui::EndTabItem();
        }
    }
    ImGui::EndTabBar();

    ImGui::PopStyleVar();
    ImGui::End();
}

void CEditor::drawSystemView()
{
    static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
    if(ImGui::CollapsingHeader("Performance"))
    {
        
        for(auto& entry : IEngineTime->profilers)
        {
           
            entry.second->DisplayForEditor(UI_W, UI_H);
           
        }
    }

    if(ImGui::CollapsingHeader("Graphics"))
    {
        ImGui::Text("==Rendering==");
        ImGui::Text("drawing @ [%d x %d], blurscale x%d @ [%d x %d]", SCREEN_WIDTH, SCREEN_HEIGHT, BLUR_SCALE, SCREEN_WIDTH / BLUR_SCALE, SCREEN_HEIGHT / BLUR_SCALE);
        ImGui::Text("upscaling to {%d x %d}, using %d threads", smith_renderer->GetFullWidth(), smith_renderer->GetFullHeight(), smith_renderer->thread_count);
        static auto RenderProfiler =  IEngineTime->GetProfiler("Render::LoopWolf()");
        RenderProfiler->DisplayForEditor(UI_W, UI_H);


        ImGui::Text("==BLUR==");
        ImGui::Text((smith_renderer->m_bBlurMethod) ? "Using Gauss" : "Using MovingAvg" );
        ImGui::Checkbox("Gauss Blur", &smith_renderer->m_bBlurMethod);

        static auto BlurProfiler =  IEngineTime->GetProfiler("Render::Blur()");
       BlurProfiler->DisplayForEditor(UI_W, UI_H);
        if(smith_renderer->m_bBlurMethod)
        {
            static float sig = smith_renderer->sigma;
            if(ImGui::Button("rebuild gauss kernel")){
                smith_renderer->GenerateGaussKernel();
                smith_renderer->sigma = sig;
                sig = smith_renderer->sigma;
            }
            
            ImGui::InputFloat("sigma",  &sig, 0.2f, 1.0f); 
            ImGui::InputInt("size", &smith_renderer->kernelSize);
        }
        else
        {
            ImGui::InputInt("size", &smith_renderer->avg_kernelSize);
        }
        
    }
  
}

void CEditor::TexturePicker(const char *title, tile_t *selectedTile, texture_t *&selectedTexture,
                            SDL_Texture *&previewTexture, std::string &preview, ImGuiTextFilter *filter, uint8_t updatetype)
{
    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");

    if (ImGui::BeginCombo(title, preview.c_str(), ImGuiComboFlags_HeightLarge))
    {
        for (const auto &entry : texture_info)
        {
            if (!filter->PassFilter(entry.first.c_str()))
                continue;
            ImGui::PushID(&entry);
            std::string name = entry.first;
            if (ImGui::Selectable(name.c_str(), selectedTexture == entry.second.texture))
            {
                selectedTexture = entry.second.texture;
                preview = ITextureSystem->FilenameFromHandle(selectedTexture->m_handle);
                previewTexture = entry.second.texture_preview;
                if (updatetype != (uint8_t)-1)
                    selectedTile->UpdateTexture(selectedTexture, (Tile_Texture)updatetype);
                // m_texLastSelected = selectedTexture;
            }
            ImGui::SameLine();
            ImGui::Image(entry.second.texture_preview, ImVec2(32, 32));
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
}

void CEditor::drawAnimView()
{

    static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");
    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
     static auto IAnimationSystem = engine->CreateInterface<CAnimationSystem>("IAnimationSystem");
     static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
    static SDL_Texture* anim_preview = NULL;

    static bool isPlaying = false;
    static auto lastPlayTime = IEngineTime->GetCurLoopTick();


    if(ImGui::Button("Save Anim. Data")){
        IResourceSystem->SaveAnimations();
    } ImGui::SameLine();
    if(ImGui::Button("Load Anim. Data")){
        IResourceSystem->LoadAnimations();
    }

    static CAnimData* selectedAnim = nullptr;
    static SDL_Texture* selectedAnimTexture = NULL;
    static editor_texture_t* selectedTextInfo = nullptr;

    static SDL_Texture* previewTexture = NULL;
    static int selectedFrame = 0;

    static std::string addNew_name = "";
     static std::string addNew_text = "";

     static ImGuiTextFilter anim_filter;
    ImGui::Columns(3, "####animeditorcols");
    anim_filter.Draw("anim filter", UI_W / 5); ImGui::SameLine();
    if(ImGui::SmallButton("x##clearfilter")) anim_filter.Clear();

    if(ImGui::BeginListBox("Animations", ImVec2(UI_W / 8, UI_H / 2))){

        for(auto& anim : IAnimationSystem->animation_list)
        {
            if(!anim_filter.PassFilter(anim.first.c_str())) continue;
            ImGui::PushID(anim.second);
            if(ImGui::Selectable(anim.first.c_str(), selectedAnim == anim.second)){
                selectedFrame = 0;
                if(previewTexture != NULL){
                    SDL_DestroyTexture(previewTexture);
                    previewTexture = NULL;
                }
                
                selectedAnim = anim.second;
                auto search = texture_info.find(anim.second->m_szTextureName);
                if(search != texture_info.end()){
                    selectedAnimTexture = search->second.texture_preview;
                    selectedTextInfo = &search->second;
                    previewTexture = SDL_CreateTexture(m_renderer, SMITH_PIXELFMT, SDL_TEXTUREACCESS_STREAMING, selectedAnim->m_size.x, selectedAnim->m_size.y);
                }
                else{
                    engine->warn("no texture %s for %s", anim.second->m_szTextureName.c_str(), anim.first.c_str());
                    selectedAnim = nullptr;
                }
                
                    
            }
            ImGui::PopID();
        }

        ImGui::EndListBox();
    }
   
    ImGui::InputText("name:", &addNew_name);
    ImGui::InputText("texture:", &addNew_text);
    static int addNew_size[2]{};
    ImGui::InputInt2("size", addNew_size);
    
    static bool dup_from = false;
    ImGui::Checkbox("copy selected", &dup_from);
    if(ImGui::Button("add seq")){
         auto ad  = new CAnimData(addNew_name);
        ad->m_szTextureName = addNew_text;
        ad->m_maskColor = Color(65, 176, 70, 255);
        ad->m_maskColorAlt = Color(65, 176, 70, 255);
        ad->m_rate = 3;
        ad->m_pos = {0, 0};
        ad->m_size = {addNew_size[0], addNew_size[1]};
        ad->AddFrame(0, {0, 0, 0, 0});
       
        if(!IAnimationSystem->animation_list.emplace(addNew_name, ad).second){
            engine->warn("failed to add anim %s from editor", addNew_name.c_str() ); 
            delete ad;
        }
    }

    ImGui::NextColumn();
    if(selectedAnim == nullptr || selectedAnimTexture == NULL) {  ImGui::NextColumn(); ImGui::Columns(); return; }
    static bool scalex2 = false; static int edit_scale = 1; if(edit_scale < 1) edit_scale = 1; static Color rect_clr = Color::White();
    static bool prev_other = false;
    if(ImGui::CollapsingHeader("view options")){
        ImGui::Checkbox("scale preview x2", &scalex2); ImGui::SameLine(); 
         ImGui::Checkbox("show prev on other col", &prev_other); 
        ImGui::SliderInt("edit scale", &edit_scale, 1, 5);
        rect_clr = Editor::colorPicker("rect color", rect_clr, ImGuiColorEditFlags_NoAlpha);
    }
    
    auto& frame = selectedAnim->m_frames.at(selectedFrame);

    ImGui::Text("%s | src[%d x %d]", selectedAnim->m_szTextureName.c_str(), selectedTextInfo->texture->m_size.x, selectedTextInfo->texture->m_size.y);
    ImVec2 ip = ImGui::GetCursorScreenPos();
    ImGui::Image(selectedAnimTexture,  { selectedTextInfo->texture->m_size.x / edit_scale, selectedTextInfo->texture->m_size.y / edit_scale}); //need scale slider 
    auto draw = ImGui::GetWindowDrawList();
    draw->AddRect({ip.x + frame.m_rect.x / edit_scale, ip.y + frame.m_rect.y / edit_scale }, {ip.x + (frame.m_rect.x + frame.m_rect.w) / edit_scale, ip.y + (frame.m_rect.y + frame.m_rect.h) / edit_scale}, Editor::ColorToIU32(rect_clr) );
    
    if(!prev_other)
    {
         ImGui::Text("%s frame [%d]", selectedAnim->m_szName.c_str(), selectedFrame);

        ImVec2 frameSize = (scalex2 == true) ? ImVec2(selectedAnim->m_size.x * 2, selectedAnim->m_size.y * 2) : ImVec2(selectedAnim->m_size.x, selectedAnim->m_size.y);
        ImGui::Image(previewTexture, frameSize);
    }
   


    ImGui::NextColumn();
    

    ImGui::Text("%s {%i x %i} #%li frames", selectedAnim->m_szName.c_str(), selectedAnim->m_size.x, selectedAnim->m_size.y, selectedAnim->m_numFrames);
    if(ImGui::CollapsingHeader("edit seq data")){
        selectedAnim->m_maskColor = Editor::colorPicker("mask color", Color(selectedAnim->m_maskColor), 0);
        selectedAnim->m_maskColorAlt = Editor::colorPicker("mask color alt", Color(selectedAnim->m_maskColorAlt), 0);
        ImGui::InputInt("frametime", &selectedAnim->m_rate);
        ImGui::InputInt2("framesize", &selectedAnim->m_size.x);

        static auto flag_names = magic_enum::enum_entries<AnimFlags>();
     
        static int selected_flag = 0;
        if(ImGui::BeginListBox("Flags", ImVec2(UI_W / 9, UI_H / 12))){
            for(int i = 0; i < (int)flag_names.size(); ++i)
            {
                if(flag_names[i].first == AnimFlags_SIZE) continue;
                ImGui::PushID(&flag_names[i]);

                if(ImGui::Selectable(flag_names[i].second.data(), (selectedAnim->m_flags &  flag_names[i].first))){
                     selectedAnim->m_flags = (selectedAnim->m_flags &  flag_names[i].first) ?  (selectedAnim->m_flags & ~flag_names[i].first) : (selectedAnim->m_flags | flag_names[i].first);
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }
      
    }
   

    if(ImGui::Button("add frame")){ //should dup rect from before it 
        auto rect = selectedAnim->GetFrames().back().m_rect;
        selectedAnim->AddFrame();
        selectedAnim->GetFrames().back().m_rect = rect;
        selectedFrame = selectedAnim->GetLastIndex();

    } ImGui::SameLine();
    if(ImGui::Button("remove frame")){
        //todo;
        engine->log("yeah no");
    }
    if(ImGui::BeginListBox("Frames", ImVec2(UI_W / 8, UI_H / 10))){

        for(auto& frame : selectedAnim->m_frames)
        {
            ImGui::PushID(&frame);
            if(ImGui::Selectable(std::to_string(frame.m_index).c_str(), selectedFrame == frame.m_index)){
               selectedFrame = frame.m_index;     
                SDL_Surface* surf;
                SDL_LockTextureToSurface(previewTexture, NULL, &surf );
                SDL_SetSurfaceColorKey(surf, SDL_TRUE, selectedAnim->m_maskColor);
                SDL_BlitSurfaceScaled(selectedTextInfo->texture->m_texture, &(frame.m_rect), surf, NULL); 
                SDL_UnlockTexture(previewTexture);
            }
            ImGui::PopID();
        }

        ImGui::EndListBox();
    }
    if(ImGui::SmallButton("<<")) {
        if(selectedFrame == 0) selectedFrame = selectedAnim->GetLastIndex();
        else selectedFrame--;
    }
    ImGui::SameLine();
    if(ImGui::SmallButton(">")){
        isPlaying = true;
        lastPlayTime = IEngineTime->GetCurLoopTick();
    } ImGui::SameLine();
    if(ImGui::SmallButton("=")) { isPlaying = false; }
    ImGui::SameLine();
    if(ImGui::SmallButton(">>")) {
        if(selectedFrame == selectedAnim->GetLastIndex()) selectedFrame = 0;
        else selectedFrame++;
    }
    if(isPlaying){
        auto curTick = IEngineTime->GetCurLoopTick();
        if(curTick > (lastPlayTime + selectedAnim->GetRate())){
            
            frame = selectedAnim->m_frames.at(selectedFrame);
            SDL_Surface* surf;
            SDL_LockTextureToSurface(previewTexture, NULL, &surf );
            SDL_FillSurfaceRect(surf, NULL, 0);
            SDL_BlitSurfaceScaled(selectedTextInfo->texture->m_texture, &(frame.m_rect), surf, NULL); 
            SDL_UnlockTexture(previewTexture);
            if(selectedFrame == selectedAnim->GetLastIndex()) selectedFrame = 0;
            else selectedFrame++;
            lastPlayTime = curTick;
        }
    }
    ImGui::SeparatorText("edit frame");
    if(ImGui::Button("update preview")){
        SDL_Surface* surf;
        SDL_LockTextureToSurface(previewTexture, NULL, &surf );
        SDL_SetSurfaceColorKey(surf, SDL_TRUE, selectedAnim->m_maskColor);
        SDL_BlitSurfaceScaled(selectedTextInfo->texture->m_texture, &(frame.m_rect), surf, NULL); 
        SDL_UnlockTexture(previewTexture);
        
    }
    ImGui::InputInt4("Rect", &frame.m_rect.x);

    ImGui::SeparatorText("preview");
    if(prev_other)
    {
         ImGui::Text("%s frame [%d]", selectedAnim->m_szName.c_str(), selectedFrame);

        ImVec2 frameSize = (scalex2 == true) ? ImVec2(selectedAnim->m_size.x * 2, selectedAnim->m_size.y * 2) : ImVec2(selectedAnim->m_size.x, selectedAnim->m_size.y);
        ImGui::Image(previewTexture, frameSize);
    }
    ImGui::Columns(); 
}