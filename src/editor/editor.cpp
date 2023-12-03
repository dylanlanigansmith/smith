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

    texture_info.clear(); // GIANT MEMORY LEAK!!
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
    static bool changedLastFrame = false;
    static auto IInputSystem = engine->CreateInterface<CInputSystem>("IInputSystem");
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
    auto player = IEntitySystem->GetLocalPlayer();
    if (auto draw = ImGui::GetBackgroundDrawList(); draw) // just for scope really
    {
        static constexpr auto text_color = IM_COL32(255, 255, 255, 255);

        if (settings.show_pos)
        {
            auto p = player->GetPosition();
            std::string str_playerpos = Util::stringf("(%.1f, %.1f, %.1f)", p.x, p.y, p.z);
            auto textSize = ImGui::CalcTextSize(str_playerpos.c_str());
            static const ImVec2 position(SCREEN_WIDTH_FULL - 10, SCREEN_HEIGHT_FULL - 10);
            draw->AddText(position - textSize, text_color, str_playerpos.c_str());
        }
        if (settings.show_cam)
        {
            auto c = player->Camera().m_vecPlane;
            auto d = player->Camera().m_vecDir;
            std::string str_camplane = Util::stringf("(%.1f, %.1f) | (%.1f, %.1f)", c.x, c.y, d.x, d.y);
            auto camtextSize = ImGui::CalcTextSize(str_camplane.c_str());
            static const ImVec2 camposition(SCREEN_WIDTH_FULL - 10, SCREEN_HEIGHT_FULL - 25);
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
    }

    if (!isOpen())
        return;
    SDL_SetRelativeMouseMode(SDL_FALSE);

    if (!m_bHasInit)
        Init();
    ImVec2 windowSize(UI_W, UI_H);
    ImVec2 windowPos(0, 0);
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("SmithEditor v0.0.0");

    // the code here can be a bit greasy because it is a dev tool but try not to make it too singletrack
    if (ImGui::BeginTabBar("###mode"))
    {
        if (ImGui::BeginTabItem("Map View"))
        {
            drawMapView();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Resources"))
        {
            drawResourceView();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Scene View"))
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
    static ImGuiTextFilter textureFilter;
    ImGui::Columns(2, "###mapedit");
    ImVec2 tileSize(20, 20);
    for (auto &row : world)
    {
        for (auto &tile : row)
        {
            ImGui::PushID(tile.id);
            hTexture tile_handle = tile.m_pTexture->m_handle;

            hTexture color_handle = (tile.m_nType == Level::Tile_Wall) ? tile_handle : tile.m_hTextureFloor;
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
                primary_texture_preview = texturename;
                previewTexture = text_info.texture_preview;
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

    if (selectedTile->m_nType == Level::Tile_Empty)
    {
        TexturePicker("Ceiling", selectedTextureCeiling, previewTexture, ceiling_texture_preview, &textureFilter);
        TexturePicker("Floor", selectedTextureFloor, previewTexture2, floor_texture_preview, &textureFilter);
        if (selectedTextureCeiling != nullptr && previewTexture != NULL)
        {
            auto strCeil = ITextureSystem->FilenameFromHandle(selectedTextureCeiling->m_handle);

            previewTexture = texture_info.at(strCeil).texture_preview;
            ImGui::Image(previewTexture, ImVec2(64, 64));
            // selectedTile->UpdateTexture(selectedTextureCeiling, TileTexture_Ceiling);
        }
        if (selectedTextureFloor != nullptr && previewTexture2 != NULL)
        {
            auto strFloor = ITextureSystem->FilenameFromHandle(selectedTextureFloor->m_handle);

            previewTexture2 = texture_info.at(strFloor).texture_preview;
            if (selectedTextureCeiling != nullptr && previewTexture != NULL)
                ImGui::SameLine();
            ImGui::Image(previewTexture2, ImVec2(64, 64));
            //  selectedTile->UpdateTexture(selectedTextureFloor, TileTexture_Floor); //not too happy about how ceil/flr ended up, shoulda just been 2 textures
        }
    }
    else
    {
        TexturePicker("Wall", selectedTexture, previewTexture, primary_texture_preview, &textureFilter);
        if (selectedTexture != nullptr && previewTexture != NULL)
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
    TexturePicker("Paint", paintTexture, paintpreviewTexture, paint_texture_preview, &textureFilter);
    if (paintTexture != nullptr && paintpreviewTexture != NULL)
    {
        ImGui::Image(paintpreviewTexture, ImVec2(64, 64));
        if (ImGui::IsKeyDown(ImGuiKey_Space))
        {
            selectedTile->m_nType = Level::Tile_Wall;
            selectedTile->UpdateTexture(paintTexture);
        }
    }

    ImGui::Columns();
    // Editor::IVec2Str(tile.m_vecPosition).c_str(),
}

void CEditor::ShowEntityObject(CBaseEntity *entity, ImVec2 offset, ImDrawList *draw_list)
{
    // Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
    ImGui::PushID(entity);
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
    // Text and Tree nodes are less high than framed widgets, using AlignTextToFramePadding() we add vertical spacing to make the tree lines equal high.
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    bool node_open = ImGui::TreeNode("Entity", "%s_%u", entity->GetName().c_str(), entity->GetID());
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%s", entity->GetSubclass().c_str());
    auto pos = entity->GetPosition();
    const float GRID_STEP = 32.f;
    if (entity->IsRenderable() && !(entity->IsLocalPlayer()) && entity->GetID() > 0)
    {
        auto rdr = (CBaseRenderable *)entity;
        auto name = ITextureSystem->FilenameFromHandle(rdr->GetTextureHandle());
        auto editor_text = texture_info.at(name);
        ImGui::Text("mat: %s", name.c_str());

        float offset_x = GRID_STEP * (float)pos.x + offset.x;
        float offset_y = GRID_STEP * (float)pos.y + offset.y;
        draw_list->AddImage(editor_text.texture_preview, ImVec2(pos.x + offset_x, pos.y + offset_y), ImVec2(pos.x + offset_x + GRID_STEP, pos.y + offset_y + GRID_STEP));
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

            static const auto cbaseenemy = CEntitySystem::CreateType("CBaseEnemy");

            ImGui::TreeNodeEx("Properties", flags, "%d", i);

            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);

            ImGui::Text("Position {%.2f, %.2f, %.1f}", pos.x, pos.y, pos.z);

            //  ImGui::InputFloat("##value", &placeholder_members[i], 1.0f);
            //  else
            //  ImGui::DragFloat("##value", &placeholder_members[i], 0.01f);

            if (entity->IsRenderable() && !(entity->IsLocalPlayer()))
            {
                auto rdr = (CBaseRenderable *)entity;
                auto name = ITextureSystem->FilenameFromHandle(rdr->GetTextureHandle());
                auto editor_text = texture_info.at(name);
                ImGui::Text("mat: %s", name.c_str());
            }
            if (entity->GetType() == cbaseenemy)
            {
                auto enemy = (CBaseEnemy *)entity;
                ImGui::Text("Health {%i / %i}", enemy->GetHealth(), enemy->GetMaxHealth());

                auto path = enemy->GetPathFinder();
                if (path->HasPath())
                {
                    // engine->log("%i", path->path.size());
                    auto idx = path->m_iPathIndex;
                    int i = 0;
                    for (auto &node : path->path)
                    {

                        const float GRID_STEP = 32.f;
                        float offset_x = GRID_STEP * (float)(node.x + 0.5) + offset.x;
                        float offset_y = GRID_STEP * (float)(node.y + 0.5) + offset.y;
                        engine->log("%i %i", node.x, node.y);
                        auto col = IM_COL32(0, 240, 255, 165);
                        if (i > idx)
                            col = IM_COL32(255, 100, 0, 165);
                        draw_list->AddCircleFilled(ImVec2(node.x + offset_x, node.y + offset_y), 8.f, col, 12);
                        i++;
                    }
                }
            }
            else if (entity->IsLocalPlayer())
            {
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
    static ImVector<ImVec2> points;
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

    // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
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

    // This will catch our interactions
    // ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const bool is_hovered = ImGui::IsItemHovered();                            // Hovered
    const bool is_active = ImGui::IsItemActive();                              // Held
    const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
    const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

    // Add first and second point
    if (is_hovered && !adding_line && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        points.push_back(mouse_pos_in_canvas);
        points.push_back(mouse_pos_in_canvas);
        adding_line = true;
    }
    if (adding_line)
    {
        points.back() = mouse_pos_in_canvas;
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            adding_line = false;
    }

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
        if (adding_line)
            points.resize(points.size() - 2);
        adding_line = false;
        if (ImGui::MenuItem("Remove one", NULL, false, points.Size > 0))
        {
            points.resize(points.size() - 2);
        }
        if (ImGui::MenuItem("Remove all", NULL, false, points.Size > 0))
        {
            points.clear();
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
            }
            ImGui::PopID();
        }
    }

    // draw_list->PopClipRect();
    static bool pOpen = false;
    ImGui::Checkbox("Property Editor", &pOpen);

    ImGui::EndChild();
    ImGui::SetNextWindowSize(ImVec2(UI_W / 3.5, UI_H - 150), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(SCREEN_WIDTH_FULL * 0.7, SCREEN_HEIGHT_FULL * 0.2));
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
            ShowEntityObject(ent, ImVec2{canvas_p0.x, canvas_p0.y}, draw_list);

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
    if (ImGui::BeginCombo("Type", type_preview.c_str(), 0))
    {
        ImGui::PushID("materialrscoption");
        if (ImGui::Selectable("Materials", &view_textures))
        {
            view_textures = true;
            view_other = false;
            type_preview = "Materials";
        }
        ImGui::PopID();
        ImGui::PushID("leveldatarscoption");
        if (ImGui::Selectable("Level Data", &view_other))
        {
            view_textures = false;
            view_other = true;
            type_preview = "Level Data";
        }
        ImGui::PopID();
        ImGui::EndCombo();
    }

    if (view_textures)
        return drawMaterialView();

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

void CEditor::TexturePicker(const char *title, texture_t *&selectedTexture, SDL_Texture *&previewTexture, std::string &preview, ImGuiTextFilter *filter)
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
                // m_texLastSelected = selectedTexture;
            }
            ImGui::SameLine();
            ImGui::Image(entry.second.texture_preview, ImVec2(32, 32));
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
}