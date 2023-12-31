#include "IEngineTime.hpp"
#include <SDL3/SDL.h>



CEngineTime::~CEngineTime()
{
   
}

void CEngineTime::OnCreate()
{
   
  
   m_loopticks = 0;
   m_renderticks = 0;
   m_loopStart = GetCurTime();
   m_updateTimer.start_time = GetCurTime();
}

void CEngineTime::OnShutdown()
{
   for(auto prof : profilers){
      delete prof.second;
   }
}

void CEngineTime::OnLoopStart()
{
    m_loopTimer.start_time = GetCurTime();
    m_updateTimer.cur_time = GetCurTime();
}

void CEngineTime::OnLoopEnd()
{
   
   m_loopTimer.cur_time = GetCurTime();
   m_updateTimer.cur_time = GetCurTime();
   m_lastFrameTime = m_loopTimer.Elapsed();
   m_renderticks++;
}

float CEngineTime::GetFPS()
{
   auto sec = GetLastFrameTime().sec();

   return 1.0 / sec;
}

float CEngineTime::GetFPSAvg()
{

   static const int sampleCount = 60; // Number of frames to average over
   static float frameTimes[sampleCount] = {0.0f};
   static int frameIndex = 0;

   // Store the current frame time
   frameTimes[frameIndex] = GetLastFrameTime().sec();
   frameIndex = (frameIndex + 1) % sampleCount;

   // Calculate average frame time
   float sum = 0.0f;
   for (int i = 0; i < sampleCount; ++i) {
      sum += frameTimes[i];
   }
   float avgFrameTime = sum / sampleCount;

   
   if (avgFrameTime == 0.0f) {
      return 0.0f; 
   }

   return 1.0 / avgFrameTime;

}

Time_t CEngineTime::GetCurTime()
{
   return Time_t((time_ns_t)SDL_GetTicksNS());
}

Time_t CEngineTime::GetLastFrameTime()
{
   return m_lastFrameTime;
}

looptick_t CEngineTime::GetCurLoopTick()
{
   return m_loopticks;
}
