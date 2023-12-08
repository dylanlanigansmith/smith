#include "IResourceSystem.hpp"
#include <engine/engine.hpp>
#include <filesystem>
#include <stdlib.h>
#include <nlohmann/json.hpp>
#include <data/Texture.hpp>
#include <fstream>

CResourceSystem::~CResourceSystem()
{
}

void CResourceSystem::OnCreate()
{
    std::string home = getenv("HOME"); // should be moved to IFileSystemLinux
    if (!home.empty())
        m_szHomeDir = home;
    else
        warn("Failed to find home directory");

    m_szResourcePath = m_szHomeDir + LOG_RESOURCE_PATH;
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
    if (FileExists(full_path))
        return full_path;

    Error("resource subdir %s not found [%s]", folder.c_str(), full_path.c_str());
    return std::string();
}
std::vector<std::pair<std::string, std::string>> CResourceSystem::GetDirectoryStructure(const std::string& subdir)
{
      namespace fs = std::filesystem;
    auto path = GetResourceSubDir(subdir);
    std::vector<std::pair<std::string, std::string>> dir_structure;
    for (auto const &dir_entry : fs::recursive_directory_iterator(path))
    {
        if(dir_entry.is_directory()) continue;
        std::string file = FindSubdirFromPath(StripResourcePath(dir_entry.path()));
        std::string name = dir_entry.path().filename();
       std::pair<std::string, std::string> to_add = {name, file};
 
        dir_structure.push_back(to_add);
    }
    std::sort(dir_structure.begin(), dir_structure.end(), [](const std::pair<std::string, std::string>& a, const std::pair<std::string, std::string>& b) {
        return a.second < b.second; // This will sort by subdirectory
    });
    return dir_structure;
}
std::string CResourceSystem::FindSubdirFromPath(const std::string& path)
{
    if(path.find_first_of("/") == std::string::npos) //might not be in a subdir
        return std::string();
    auto rootandsub =  path.substr(0, path.find_last_of("/")) ; //this stage is ex. "/material/nature"
    return rootandsub.substr(rootandsub.find_last_of("/") + 1); // this is "nature"
  
}
std::string CResourceSystem::StripResourcePath(const std::string& path)
{
    int len = m_szResourcePath.length();
    return path.substr(len);
}
std::string CResourceSystem::FindResourceFromPath(const std::string &path, const std::string &name)
{
    namespace fs = std::filesystem;

    for (auto const &dir_entry : fs::recursive_directory_iterator(path))
    {
        std::string file = std::string(dir_entry.path());
        if (file.find(name) != std::string::npos)
            return file;
    }

    return std::string();
}

std::string CResourceSystem::FindResource(const std::string &subdir_path, const std::string &name)
{
    std::string ret = MergePathAndFileName(subdir_path, name);
    if (FileExists(ret))
        return ret;
    ret = std::string(); // why?
    ret = FindResourceFromPath(subdir_path, name);
    if (ret.empty())
    {
        Error("unable to locate %s under subdir %s ", name.c_str(), subdir_path.c_str());
    }

    return ret;
}

inline std::string CResourceSystem::MergePathAndFileName(const std::string &path, const std::string &name)
{
    std::string full_path = path + name;
    return full_path;
}

bool CResourceSystem::FileExists(const std::string &path)
{
    if (std::filesystem::exists(path))
        return true;
    return false;
}

/*
ideally we have serializer objects we can just use universally


*/
#define TEX_DEF_SUBDIR "definition"
#define TEX_DEF_FILE "material.json"

void CResourceSystem::OnEngineInitFinish()
{
    
}

void CResourceSystem::OnResourceLoadStart()
{
    LoadTextureDefinition();
}

bool CResourceSystem::LoadTextureDefinition()
{
    auto dir = GetResourceSubDir(TEX_DEF_SUBDIR);
    auto path = MergePathAndFileName(dir, TEX_DEF_FILE);

    int amt = 0;
    json tex_def = ReadJSONFromFile(path);

    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
    for (const auto &key : tex_def)
    {
        CTexture toLoad;
        if (toLoad.FromJSON(key))
        {
            if (ITextureSystem->LoadFromDefinition(toLoad))
                amt++;
        }
    }
    status("loaded %i texture definitions for a db size of  %i ", amt, ITextureSystem->texture_db.size());
    return (amt > 0);
}

bool CResourceSystem::SaveTextureDefinition()
{
    static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
    json tex_def = json::object();
    int amt = 0;
    for (const auto &entry : ITextureSystem->texture_db)
    {
        auto text = entry.second;
        auto filename = ITextureSystem->FilenameFromHandle(entry.first);
        CTexture toSave(text);
        toSave.m_szName = filename;
        auto j = toSave.ToJSON();
        tex_def.emplace(filename, j);
        amt++;
    }
    auto dir = GetResourceSubDir(TEX_DEF_SUBDIR);
    auto path = MergePathAndFileName(dir, TEX_DEF_FILE);

   WriteJSONToFile(tex_def, path);

    warn("saved definitions for %i textures", amt);

    return (amt > 0);
}

bool CResourceSystem::WriteJSONToFile(const json &data, const std::string &path)
{
    try
    {
        std::ofstream file_out(path);

        file_out << std::setw(4) << data << std::endl;

        file_out.close();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

json CResourceSystem::ReadJSONFromFile(const std::string &path)
{
    json j_in;
    try
    {
        std::ifstream file_in(path);
        file_in >> j_in;
        file_in.close();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return json();
    }
    return j_in;
}

#define LEVEL_SUBDIR TEX_DEF_SUBDIR

bool CResourceSystem::LoadLevel(const std::string &name)
{
     static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    auto dir = GetResourceSubDir(LEVEL_SUBDIR);
    auto path = MergePathAndFileName(dir, AddExtension(name)); //findresource

    json j = ReadJSONFromFile(path);
    if(j.empty()) {
        Error("json object for level %s empty", name.c_str());   
        return false;
    }
    ILevelSystem->m_Level = new CLevel();

    if(ILevelSystem->m_Level->FromJSON(j)){
        status("loaded level %s from file", name.c_str());
        return true;
    }

    Error("error loading %s from %s", name.c_str(), path.c_str());
    return false;
}

bool CResourceSystem::SaveLevel()
{
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    if(ILevelSystem->m_Level ==  nullptr) return false;
  
    auto& level = ILevelSystem->m_Level;
    auto j = level->ToJSON();
    auto dir = GetResourceSubDir(LEVEL_SUBDIR);
    auto path = MergePathAndFileName(dir, AddExtension(level->getName()));

    if( WriteJSONToFile(j, path)){
        warn("wrote level %s to file", level->getName().c_str()); return true;
    }

    log("error writing %s", path.c_str()); return false;

}