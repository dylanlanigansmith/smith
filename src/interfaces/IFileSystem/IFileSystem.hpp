#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <SDL3/SDL.h>


using dir_structure_t = std::vector<std::pair<std::string, std::string>> ; 

/*
ToDo: strip all functionality from IResource and move here now that this does the work


Pretty sure std::filesystem works on macos since cpp17 
*/

class CFileSystem : public CBaseInterface
{
    friend class CEditor;
public:
    CFileSystem() : CBaseInterface("IFileSystem") { }
    ~CFileSystem() override;
    virtual void OnCreate() override;
    virtual void OnShutdown() override;
    virtual void OnLoopStart() override {}
    virtual void OnLoopEnd() override {}
    virtual void OnRenderStart() override{}
    virtual void OnRenderEnd() override {}

    inline auto& GetLogPath() const { return m_logpath; }
    inline auto& GetResourcePath() const { return m_resourcepath; }
    inline auto& GetBasePath() const { return m_basepath; }


    virtual bool FileExists(const std::string& path);
    
    virtual dir_structure_t GetStructureAt(const std::string& path);

    inline bool ValidatePath(const std::string &path) { return path.at(path.length() - 1) == m_slash.at(0); }
    inline std::string MergePathAndFileName(const std::string &path, const std::string &name){return path + name;}
    inline std::string AddExtension(const std::string& name, const std::string& ext = ".json"){return std::string(name).append(ext);    }
    std::string GetExtension(const std::string& name );

    std::string FindSubdirFromPath(const std::string& path);
    std::string StripPath(const std::string& path, const std::string& base = "");

    std::string FindRecursively(const std::string &path, const std::string &filename);

    inline std::string MakePath(const std::string& path, const std::string& subdir){
        return path + subdir + m_slash;
    }
    inline std::string MakePath(const std::string& subdir){
        return m_basepath + subdir + m_slash;
    }

private:
    std::string m_basepath;
    std::string m_resourcepath;
    std::string m_logpath;

    std::string m_slash;
    std::string m_resource_dir_name;
    std::string m_log_dir_name;

};
