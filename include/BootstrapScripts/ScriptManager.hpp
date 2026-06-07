#pragma once
#include <algorithm>
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
        std::string run;
        std::vector<AppDataDirectory> appdirectories;
        std::vector<std::string> logfolders;
        std::vector<std::string> capabilities;
    };

    extern std::vector<BootstrapScript> LoadedScripts;

    std::string GetScriptsDir();
    std::string GetScriptPath(const std::string &title);

    // reads metadata table from lua file and fills a BootstrapScript
    bool ReadMetadata(const std::string &luaFilePath, BootstrapScript &out);

    bool InstallScript(const std::string &luaFilePath);
    bool UninstallScript(const std::string &title);

     // just scans the scripts dir and reads metadata from each bootstrap script
    void LoadScripts(bool reinstallURIs);

    inline bool HasCapability(const BootstrapScript &script, const std::string &cap) {
        return std::find(script.capabilities.begin(), script.capabilities.end(), cap) != script.capabilities.end();
    }

    // returns a bootstrap script struct by it's name
    BootstrapScript* FindFirstScriptByName(std::string);
}
