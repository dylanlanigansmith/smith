#include "IFileSystem.hpp"
#include <engine/engine.hpp>
#include <plat/platform.hpp>
#include <filesystem>
#include <cctype>
CFileSystem::~CFileSystem()
{
}

void CFileSystem::OnCreate()
{
    m_resource_dir_name = "resource";
    m_log_dir_name = "logs";
    
    m_basepath = PLATFORM.GetExecutableCWD(); //ends with slash
    m_slash = PLATFORM.GetFileSystemSlash();

    if(PLATFORM.isDeveloperMode() && PLATFORM.IsLinux()){
        std::string home = getenv("HOME"); 
        if (home.empty())
            warn("DevMode: Failed to find home directory");
        else
            m_basepath = home + DEV_BASEPATH;
        dbg("overriding base path due to dev mode");
    }
    m_resourcepath = MakePath(m_resource_dir_name);
    m_logpath = MakePath(m_log_dir_name);

    

   dbg("resource path = %s", m_resourcepath.c_str());
}

void CFileSystem::OnShutdown()
{
}

bool CFileSystem::FileExists(const std::string &path)
{
    if (std::filesystem::exists(path))
        return true;
    return false;
}

dir_structure_t CFileSystem::GetStructureAt(const std::string &path)
{
     namespace fs = std::filesystem;
    std::vector<std::pair<std::string, std::string>> dir_structure;
    for (auto const &dir_entry : fs::recursive_directory_iterator(path))
    {
        if(dir_entry.is_directory()) continue;
        std::string file = FindSubdirFromPath(StripPath(dir_entry.path(), m_resourcepath));
        std::string name = dir_entry.path().filename();
       std::pair<std::string, std::string> to_add = {name, file};
 
        dir_structure.push_back(to_add);
    }
    std::sort(dir_structure.begin(), dir_structure.end(), [](const std::pair<std::string, std::string>& a, const std::pair<std::string, std::string>& b) {
        return a.second < b.second; // This will sort by subdirectory
    });
    return dir_structure;
}

std::string CFileSystem::GetExtension(const std::string &name)
{
    auto ext =  name.substr(name.find_last_of(".") );    
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
    return ext;
}

std::string CFileSystem::FindSubdirFromPath(const std::string &path)
{
     if(path.find_first_of("/") == std::string::npos) //might not be in a subdir
        return std::string();
    auto rootandsub =  path.substr(0, path.find_last_of("/")) ; //this stage is ex. "/material/nature"
    return rootandsub.substr(rootandsub.find_last_of("/") + 1); // this is "nature"
}

std::string CFileSystem::StripPath(const std::string &path, const std::string &base)
{
    std::string strip = (base.empty()) ? m_basepath : base;
    return path.substr(strip.length());
}

std::string CFileSystem::FindRecursively(const std::string &path, const std::string &filename)
{
    namespace fs = std::filesystem;

    for (auto const &dir_entry : fs::recursive_directory_iterator(path))
    {
        std::string file = std::string(dir_entry.path());
        if (file.find(filename) != std::string::npos)
            return file;
    }

    return std::string();
}
