#pragma once

#include <string>
#include <vector>

namespace ScatterManager {
    struct Scatter {
        bool Active = true;
        std::string RobloxURI;
        std::vector<std::string> RobloxURIs;
        std::string RobloxRunCommand;
        std::string RobloxAppDataDirectory;
        std::string ScatterTitle;
    };
    
    extern std::vector<Scatter> LoadedScatters;

    // this function returns a heap allocated scatter struct, give it a scatter json string.
    Scatter* ParseScatter(std::string);

    enum ScatterInstallResult {
        SINSTALLSUCCESS,
        SINSTALLUNKNOWNENVIRONMENT,
    };

    // this function installs the scatter appropriately, it takes in a scatter struct pointer.
    ScatterInstallResult InstallScatter(Scatter*);

    enum ScatterUninstallResult {
        SUNINSTALLSUCCESS
    };

    ScatterUninstallResult UninstallScatter(Scatter*);
}
