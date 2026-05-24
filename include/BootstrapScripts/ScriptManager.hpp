#pragma once
#include <string>
#include <vector>

namespace ScriptManager {
    struct AppDataDirectory {
        std::string path;
        std::string label;
    };

    struct BootstrapScript {
        std::string title;
        std::vector<std::string> uris;
        std::vector<std::string> platform;
        std::vector<std::string> required;
        std::string run;
        std::vector<AppDataDirectory> appdirectories;
    };

    extern std::vector<BootstrapScript> LoadedScripts;

    std::string GetScriptsDir();
    std::string GetScriptPath(const std::string &title);

    // reads metadata table from lua file and fills a BootstrapScript
    bool ReadMetadata(const std::string &luaFilePath, BootstrapScript &out);

    bool InstallScript(const std::string &luaFilePath);
    bool UninstallScript(const std::string &title);

     // just scans the scripts dir and reads metadata from each bootstrap script
    void LoadScripts();
}