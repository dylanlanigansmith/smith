#include "editor.hpp"
#include <engine/engine.hpp>
#include <imgui.h>
#include <SDL3/SDL.h>

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
    ImGui::ShowDemoWindow();
    
}
