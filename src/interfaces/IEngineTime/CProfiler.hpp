#pragma once

#include <common.hpp>
#include <logger/logger.hpp>
#include <types/CTime.hpp>
#include <util/misc.hpp>
class CEngineTime;


class CProfiler : public CLogger
{
    friend class CEngineTime;
    friend class CEditor;
public: 
    CProfiler(CEngineTime* IEngineTime, const std::string& m_szProfilerName, const std::size_t samples = 60) : 
        CLogger(this, m_szProfilerName),IEngineTime(IEngineTime), m_sampleCount(samples), m_szProfilerName(m_szProfilerName) {
            m_timeMin.set(0xFFFFFFFFFFFFFFFFull);
            m_timeMax.set(0);
             index = 0;
             m_bShowInEditor = true;
        }
    
    void Start();
    void End();

  
    std::string GetAvgString(){
        auto avg = GetAvg();
        return Util::stringf("%s: Avg. (%li ns) (%li us) (%i ms) (%f sec)", m_szProfilerName.c_str(),
        avg.ns(), avg.us(), avg.ms(),avg.sec());
    }
     std::string GetString(){
        auto avg = GetAvg();
        return Util::stringf("%s: Avg: { (%li us) (%li ms) }  Max:  (%li us)  | Min: (%li us)", m_szProfilerName.c_str(),
         avg.us(), avg.ms(), GetMax().us(), GetMin().us());
    }
    Time_t GetAvg() const{
         // Calculate average frame time
        time_ns_t sum = 0;
        for (int i = 0; i < m_sampleCount; ++i) {
            
            sum +=  rolling_values[i].ns();
        }
        time_ns_t avg = sum / m_sampleCount;

         return Time_t(avg);
    }
    Time_t GetMax() const{
        return m_timeMax;
    }
    Time_t GetMin() const{
        return m_timeMin;
    }
    Time_t ResetData(){
        m_timeMin.set(0xFFFFFFFFFFFFFFFFull);
        m_timeMax.set(0);
        for(auto time : rolling_values){
            time = Time_t();
        }
    }
    void EndAndLog(){
        End();
        warn("last time (%li ns) (%li us) (%li ms) (%f sec)", m_lastTime.ns(), m_lastTime.us(), m_lastTime.ms(), m_lastTime.sec());
    }

    const auto& History() const { return rolling_values; }
    const auto SampleCount() const { return rolling_values.size();} //not even used bc array 

    void HideFromEditor() { m_bShowInEditor = false; }
protected:

    void DisplayForEditor(int ui_w, int ui_h);

private:
    
    const int m_sampleCount;
    int index;
    CEngineTime* IEngineTime;
    std::string m_szProfilerName;
    Timer_t m_timer;
    Time_t m_lastTime;
    Time_t m_timeMin;
    Time_t m_timeMax;
    std::array<Time_t, 60> rolling_values;

    bool m_bShowInEditor;
};