#pragma once
#include "UserInterface/BootstrapWindow.hpp"
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

        bool Failed;
        bool UpdateAvailable;
        std::string FailReason;

        float DownloadProgress = 0.0f;
        long long DownloadedBytes = 0;
        long long TotalBytes = 0;
        int CompletedDownloads = 0;

        double SpeedBytesPerSecond = 0.0;
        double EstimatedSecondsRemaining = 0.0;
        TokenMap Tokens; // keep the full token map too for AfterDownload expansion
    };
    
    MainStartResult StartStrappin(ScatterManager::Scatter*, char* URI, BootstrapWindow* window = nullptr);
}