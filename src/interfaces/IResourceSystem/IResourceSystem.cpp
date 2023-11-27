#include "IResourceSystem.hpp"
#include <filesystem>
#include <stdlib.h>
CResourceSystem::~CResourceSystem()
{
}

void CResourceSystem::OnCreate()
{
    std::string home = getenv("HOME"); //should be moved to IFileSystemLinux
    if(!home.empty())
        m_szHomeDir = home;
    else log("Failed to find home directory");

    m_szResourcePath = m_szHomeDir +  "/code/smith/resource";
    log("using %s as resource folder", m_szResourcePath.c_str());
}

void CResourceSystem::OnShutdown()
{
    
}

std::string CResourceSystem::GetResourcePath()
{
    return m_szResourcePath;
}

std::string CResourceSystem::GetResourceSubDir(const std::string &folder)
{
    std::string full_path = m_szResourcePath + std::string("/").append(folder) + std::string("/");
    if(std::filesystem::exists(full_path))
        return full_path;
    
    log("resource subdir %s not found [%s]", folder.c_str(), full_path.c_str());
    return std::string();
}
