#pragma once

#include <map>
#include <string>
#include <vector>

namespace ScatterManager {
    struct BootstrapInitRequest {
        std::string url;
        std::string token;
        std::map<std::string, std::string> headers;
    };

    struct BootstrapInitCommand {
        std::string system;
        std::string token;
    };

    enum class BootstrapParseError {
        BPARSE_OK,
        BPARSE_VERSION_COUNT_MISMATCH,
        BPARSE_MISSING_TOKEN,
        BPARSE_INVALID_CURRENT_VERSION,
        BPARSE_INVALID_LATEST_VERSION,
    };

    struct BootstrapParseResult {
        BootstrapParseError error = BootstrapParseError::BPARSE_OK;
        std::string message; // this is uh yes
    };

    struct BootstrapVersionEntry {
        std::string system;  // command to run
        std::string url;     // url to fetch (for only BootstrapDownload->LatestVersion)
        std::string file;    // output file for yes
        std::string token;   // token name to store result as
    };

    struct BootstrapDownloadEntry {
        std::string url;
        std::string out;
    };

    struct BootstrapDownload {
        std::vector<BootstrapInitRequest> InitRequests;
        std::vector<BootstrapInitCommand> InitCommands;
        std::vector<BootstrapInitRequest> PreDownloadRequests;
        std::vector<BootstrapInitCommand> PreDownloadCommands;
        std::vector<BootstrapVersionEntry> CurrentVersion;
        std::vector<BootstrapVersionEntry> LatestVersion;
        std::vector<BootstrapDownloadEntry> Download;
        std::vector<std::string> AfterDownload;
    };

    struct AppDataDirectory {
        std::string path;
        std::string label;
    };
    
    struct Scatter {
        bool Active = true;
        std::string RobloxURI;
        std::vector<std::string> RobloxURIs;
        std::string RobloxRunCommand;
        std::string ScatterTitle;
        std::vector<AppDataDirectory> AppDataDirectories;
        std::vector<std::string> Required; // required programs to be in path or whatever, the name sucks
        std::vector<std::string> Platform;
        BootstrapDownload Bootstrap;
        bool HasBootstrap = false;
        BootstrapParseResult BootstrapError;
    };
    
    extern std::vector<Scatter> LoadedScatters;

    // this function returns a heap allocated scatter struct, give it a scatter json string.
    Scatter* ParseScatter(std::string);

    enum ScatterInstallResult {
        SINSTALLSUCCESS,
        SINSTALLUNKNOWNENVIRONMENT,
        SINSTALLUNSUPPORTEDPLATFORM,
        SINSTALLMISSINGREQUIRED
    };

    // this function installs the scatter appropriately, it takes in a scatter struct pointer.
    ScatterInstallResult InstallScatter(Scatter*);

    enum ScatterUninstallResult {
        SUNINSTALLSUCCESS,
        SUNINSTALLFAILED,
        SUNINSTALLUNKNOWNENVIRONMENT,
    };

    ScatterUninstallResult UninstallScatter(Scatter*);
}
