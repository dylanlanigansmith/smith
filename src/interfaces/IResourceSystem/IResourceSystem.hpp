#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <interfaces/IFileSystem/IFileSystem.hpp>
#include <data/level.hpp>

/*

Needs to be split into an IFileSystem or IPlatform sorta thing

https://wiki.libsdl.org/SDL3/SDL_GetBasePath etc.

*/



class CResourceSystem : public CBaseInterface
{
    friend class CEditor;
public:
    CResourceSystem() : CBaseInterface("IResourceSystem") { }
    ~CResourceSystem() override;
    virtual void OnCreate() override;
    virtual void OnShutdown() override;
    virtual void OnLoopStart() override {}
    virtual void OnLoopEnd() override {}
    virtual void OnRenderStart() override {}
    virtual void OnRenderEnd() override {}
    virtual void OnEngineInitFinish() override;
    virtual void OnResourceLoadStart() override;
    virtual std::string GetResourcePath();
    virtual std::string GetResourceSubDir(const std::string& folder);



    virtual bool LoadTextureDefinition();
    virtual bool SaveTextureDefinition();
    virtual bool LoadAnimations();
    virtual bool SaveAnimations();
    virtual bool LoadLevel(const std::string& name);
    virtual bool SaveLevel();

    virtual std::string FindResourceFromPath(const std::string& path, const std::string& name); //path = to subdir, returns empty string on failure
    virtual std::string FindResource(const std::string& subdir_path, const std::string& name);

    virtual std::vector<std::pair<std::string, std::string>> GetDirectoryStructure(const std::string& subdir);
private:
    std::string FindSubdirFromPath(const std::string& path);
    std::string StripResourcePath(const std::string& path);
    bool WriteJSONToFile(const json& data, const std::string& path);
    json ReadJSONFromFile(const std::string& path);
    inline std::string MergePathAndFileName(const std::string& path, const std::string& name);
    virtual bool FileExists(const std::string& path);
    std::string inline AddExtension(const std::string& name, const std::string& ext = ".json"){return std::string(name).append(ext);    }
    CFileSystem* IFileSystem;

private:
    std::string m_szResourcePath;
    std::string m_szHomeDir;
};
