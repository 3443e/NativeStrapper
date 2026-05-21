#pragma once
#include "ScatterManager.hpp"
#include <map>
#include <string>

namespace MainBootstrapper {
    enum MainStartResult {
        BOOTSTRAPSUCCESS,
        BOOTSTRAPLAUNCHERROR,
        BOOTSTRAPUPDATEERROR
    };

    using TokenMap = std::map<std::string, std::string>;

    struct BootstrapArtifact {
        std::vector<std::string> CurrentVersions;  // results in order from CurrentVersion entries
        std::vector<std::string> LatestVersions;   // results in order from LatestVersion entries
        std::vector<int> OutdatedIndices;          // which indices need updating

        bool UpdateAvailable;
        std::string FailReason;

        TokenMap Tokens; // keep the full token map too for AfterDownload expansion
    };
    
    MainStartResult StartStrappin(ScatterManager::Scatter*, char* URI);
}