#include "platform.hpp"

CPlatform::CPlatform()  : CLogger(this), m_devMode(false)
{

}

bool CPlatform::Init(int argc, char **argv)
{
    m_cmdLine.Init(argc, argv);
    if(!m_cmdLine.HasArg("release"))
        m_devMode = true;
    
    Debug(m_devMode);
    dbg("finding platform info");
    std::string error_str;
    if(!Platform::CheckSDLVersion(error_str)){
        Dialog().MessageBox( error_str, SDL_MESSAGEBOX_WARNING); //we will try anyways!
    }
    m_platName = Platform::GetPlatformName();
    m_platType = Platform::GetPlatformTypeByName(m_platName.c_str());
    if(m_platType == Platform::UNKNOWN) return false;
    dbg("init on '%s' w/ cmd '%s' and [%li] args ", m_platName.c_str(), m_cmdLine.GetCmd().c_str(), m_cmdLine.NumArgs());
    m_basePath = Platform::GetBasePath();
    if(m_basePath.empty()){
        m_basePath = "./";
        Dialog().MessageBox(SDL_MESSAGEBOX_WARNING, "Could not find base path: %s", SDL_GetError());
        if(m_platType == Platform::WIN) return false;
    }
    dbg("found base path %s,  [slash=%c]", m_basePath.c_str(), GetFileSystemSlash() );

    m_sysInfo.find();
    dbg("sys_info: [{cpu_cores: %i} {memory: %d MiB}], planning to use %d render threads", m_sysInfo.sys_cores, m_sysInfo.sys_ram, m_sysInfo.render_threads_to_use);
    dbg("platform init success!");
    return true;
}



