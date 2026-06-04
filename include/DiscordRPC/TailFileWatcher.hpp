#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace TailFileWatcher {
    struct FileInfo {
        std::filesystem::path path;
        std::uintmax_t last_pos = 0;
    };

    struct FileWatcherObj {
        std::unordered_map<std::string, std::unordered_map<std::string, FileInfo>> filesByFolder;
        std::string undealt_with;
    };

    FileWatcherObj* InitTailFileWatcher(const std::vector<std::string>& folders);
    bool TailFileWatcherDealWith(TailFileWatcher::FileWatcherObj* obj, const std::vector<std::string>& folders);
}