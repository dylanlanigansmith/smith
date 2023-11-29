#include "IEngineTime.hpp"
#include <SDL3/SDL.h>
CEngineTime::~CEngineTime()
{

}

void CEngineTime::OnCreate()
{
   m_loopticks = 0;
   m_loopStart = GetCurTime();
   m_updateTimer.start_time = GetCurTime();
}

void CEngineTime::OnShutdown()
{
}

void CEngineTime::OnLoopStart()
{
    m_loopTimer.start_time = GetCurTime();
    m_updateTimer.cur_time = GetCurTime();
}

void CEngineTime::OnLoopEnd()
{
   m_loopticks++;
   m_loopTimer.cur_time = GetCurTime();
   m_updateTimer.cur_time = GetCurTime();
   m_lastFrameTime = m_loopTimer.Elapsed();
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
