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
    IFileSystem = engine->CreateInterface<CFileSystem>("IFileSystem");
    if(!IFileSystem) {
        Error("Failed acquire IFileSystem: got 0x%lx", (uintptr_t)IFileSystem); 
    }
    const std::string res_path = IFileSystem->GetResourcePath();
    log("using %s as resource folder", res_path.c_str());

    

    m_resource_subdirs = { 
        {"material", IFileSystem->MakePath(res_path, "material")}, 
        {"audio", IFileSystem->MakePath(res_path, "audio")},
        {"definition", IFileSystem->MakePath(res_path, "definition")},
        {"level", IFileSystem->MakePath(res_path, "level")},
        };


}

void CResourceSystem::OnShutdown()
{
}

std::string CResourceSystem::GetResourcePath()
{
    return IFileSystem->GetResourcePath();
}

std::string CResourceSystem::GetResourceSubDir(const std::string &folder)
{
    auto search = m_resource_subdirs.find(folder);
    if(search != m_resource_subdirs.end()){
        return search->second;
    }
    warn("adding folder %s to resource subdir list, this is likely unintended!!", folder.c_str());
    auto full_path = IFileSystem->MakePath(IFileSystem->GetResourcePath(), folder);
    if (IFileSystem->FileExists(full_path)){
        if(!m_resource_subdirs.emplace(folder, full_path).second)
            Error("failed to add resource subdir %s / [%s] to subdir list, this is likely a double error", folder.c_str(), full_path.c_str());
        return full_path;
    }
        

    Error("resource subdir %s not found [%s]", folder.c_str(), full_path.c_str());
    return std::string();
}
std::vector<std::pair<std::string, std::string>> CResourceSystem::GetDirectoryStructure(const std::string& subdir)
{
    auto path = GetResourceSubDir(subdir);
    return IFileSystem->GetStructureAt(path);
}
std::string CResourceSystem::FindSubdirFromPath(const std::string& path)
{
    return IFileSystem->FindSubdirFromPath(path);
}

std::string CResourceSystem::StripResourcePath(const std::string& path)
{
    return IFileSystem->StripPath(path, IFileSystem->GetResourcePath());
}
std::string CResourceSystem::FindResourceFromPath(const std::string &path, const std::string &name)
{
     warn("calling deprecated function: CResourceSystem::FindResourceFromPath");
    return IFileSystem->FindRecursively(path, name);
}

std::string CResourceSystem::FindResource(const std::string &subdir_path, const std::string &name)
{
    std::string ret = IFileSystem->MergePathAndFileName(subdir_path, name);
    if (IFileSystem->FileExists(ret))
        return ret;
    ret = std::string(); // blanked out since easy path didnt work
    ret = IFileSystem->FindRecursively(subdir_path, name);
    if (ret.empty())
    {
        Error("unable to locate %s under subdir %s ", name.c_str(), subdir_path.c_str());
    }

    return ret;
}

std::string CResourceSystem::FindResourceFromSubdir(const std::string &subdir_name, const std::string &name)
{
    return FindResource(GetResourceSubDir(subdir_name), name);
}

inline std::string CResourceSystem::MergePathAndFileName(const std::string &path, const std::string &name)
{
    warn("calling deprecated function: CResourceSystem::MergePathAndFileName");
    return IFileSystem->MergePathAndFileName(path, name);
}

bool CResourceSystem::FileExists(const std::string &path)
{
   warn("calling deprecated function: CResourceSystem::FileExists");
    return IFileSystem->FileExists(path);
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
    LoadAnimations();
}

bool CResourceSystem::LoadTextureDefinition()
{
    auto dir = GetResourceSubDir(TEX_DEF_SUBDIR);
    auto path = IFileSystem->MergePathAndFileName(dir, TEX_DEF_FILE);

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
    auto path = IFileSystem->MergePathAndFileName(dir, TEX_DEF_FILE);

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
        Error("CResourceSystem::WriteJSONToFile(data, %s) : %s", path.c_str(), e.what());
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
        Error("CResourceSystem::ReadJSONFromFile(%s) : %s", path.c_str(), e.what());
        
        return json();
    }
    return j_in;
}

#define LEVEL_SUBDIR TEX_DEF_SUBDIR

bool CResourceSystem::LoadLevel(const std::string &name)
{
     static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    auto dir = GetResourceSubDir(LEVEL_SUBDIR);
    auto path = IFileSystem->MergePathAndFileName(dir, AddExtension(name)); //findresource

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
    auto path = IFileSystem->MergePathAndFileName(dir, AddExtension(level->getName()));

    if( WriteJSONToFile(j, path)){
        warn("wrote level %s to file", level->getName().c_str()); return true;
    }

    log("error writing %s", path.c_str()); return false;

}

bool CResourceSystem::SaveAnimations()
{
    static auto IAnimationSystem = engine->CreateInterface<CAnimationSystem>("IAnimationSystem");
    auto dir = GetResourceSubDir("definition");
    auto path = IFileSystem->MergePathAndFileName(dir, IFileSystem->AddExtension("animation"));

    auto j = IAnimationSystem->ListToJson();

     if( WriteJSONToFile(j, path)){
        warn("wrote anim data to file"); return true;
    }

    Error("error writing animdata %s", path.c_str()); return false;

}

bool CResourceSystem::LoadAnimations()
{
     static auto IAnimationSystem = engine->CreateInterface<CAnimationSystem>("IAnimationSystem");
    auto dir = GetResourceSubDir("definition");
    auto path = IFileSystem->MergePathAndFileName(dir, IFileSystem->AddExtension("animation"));

    json j = ReadJSONFromFile(path);
    if(j.empty()) {
        Error("json object for animdata %s empty", path.c_str());   
        return false;
    }
    note("loading animations from file");
    IAnimationSystem->ListFromJson(j);

    return true;

}