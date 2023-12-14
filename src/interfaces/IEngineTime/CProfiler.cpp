#include "IEngineTime.hpp"
#include "CProfiler.hpp"
#include <SDL3/SDL_clipboard.h>
#include <imgui.h>

void CProfiler::Start()
{
    m_timer = Timer_t(IEngineTime->GetCurTime());
}
void CProfiler::End()
{
    m_timer.Update(IEngineTime->GetCurTime());

    Time_t time = m_timer.Elapsed();
    m_lastTime = time;
    if (time > m_timeMax)
        m_timeMax = time;
    if (time < m_timeMin)
        m_timeMin = time;

    rolling_values.at(index) = time;

    index = (index + 1) % m_sampleCount;
}


void CProfiler::DisplayForEditor(int ui_w, int ui_h)
{
    if(!m_bShowInEditor) return;
       
        ImGui::PushID(this);
        ImGui::SeparatorText( m_szProfilerName.c_str());
        std::string info_str = GetAvgString();
        ImGui::Text(info_str.c_str());
        ImGui::SameLine();
        ImGui::Text(" Max %li Min %li", GetMax().us(), GetMin().us());


        const static ImVec2 plot_size = {(ui_w * 0.75f), (ui_h * 0.2f)};
        if(ImGui::CollapsingHeader("Graph")){
             float history[60];
            auto &th = rolling_values;
            for (int i = 0; i < th.size(); ++i)
            {
                history[i] = th[i].us() / 1000.f;
            }
            constexpr time_us_t scale_offset =  500;
            ImGui::PlotLines("History", history, IM_ARRAYSIZE(history), 0,
                            (const char *)__null, ( (GetMin().us() - scale_offset ) / 1000.f), ( (GetMax().us() + scale_offset) / 1000.f), plot_size );
        }
      
        if (ImGui::Button("Copy "))
        {
            auto cpy = GetString().append(" @ ");
            cpy.append(GetTime(true));
            SDL_SetClipboardText(cpy.c_str());
        }
        ImGui::PopID();
}