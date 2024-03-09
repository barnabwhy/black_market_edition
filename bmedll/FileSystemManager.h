#pragma once

#include "pch.h"
#include "IFileSystem.h"

class FileSystemManager
{
private:
    static std::regex s_mapFromVPKRegex;

    std::shared_ptr<spdlog::logger> m_logger;
    SourceInterface<IFileSystem> m_engineFileSystem;
    fs::path m_basePath;
    fs::path m_bspPath;
    fs::path m_compiledPath;
    fs::path m_dumpPath;
    fs::path m_modsPath;
    std::vector<std::string> m_mapVPKs;
    std::vector<std::string> m_mapNames;
    std::string m_lastMapReadFrom;
    bool m_requestingOriginalFile;
    bool m_blockingRemoveAllMapSearchPaths;

    void CacheMapVPKs();
    bool ShouldReplaceFile(const std::string_view& path);

public:
    FileSystemManager(const std::string& basePath);
    ~FileSystemManager();

    void EnsurePathsCreated();
    void AddSearchPathHook(IFileSystem* fileSystem, const char* pPath, const char* pathID, SearchPathAdd_t addType);
    bool ReadFromCacheHook(IFileSystem* fileSystem, const char* path, void* result);
    FileHandle_t ReadFileFromVPKHook(VPKData* vpkInfo, __int32* b, const char* filename);
    unsigned __int64 __fastcall RemoveAllMapSearchPathsHook(__int64 thisptr);
    VPKData* MountVPKHook(IFileSystem* fileSystem, const char* vpkPath);
    void AddVPKFileHook(IFileSystem* fileSystem, char const* pBasename, void* a3, bool a4, SearchPathAdd_t addType, bool a6);
    const std::vector<std::string>& GetMapNames();
    const std::string& GetLastMapReadFrom();
    void MountAllVPKs();

    bool FileExists(const char* fileName, const char* pathID);
    std::string ReadOriginalFile(const char* path, const char* pathID);

    void DumpVPKScripts(const std::string& vpkPath);
    void DumpFile(FileHandle_t handle, const std::string& dir, const std::string& path);
    void DumpAllScripts(const CCommand& args);

    const fs::path& GetBasePath();
    const fs::path& GetModsPath();
    const fs::path& GetCompilePath();
};