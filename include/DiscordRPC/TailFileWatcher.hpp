#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>

namespace TailFileWatcher {
    struct FileInfo {
        std::filesystem::path path;
        std::uintmax_t last_pos = 0;
    };

    struct FileWatcherObj {
        std::unordered_map<std::string, FileInfo> files;
        std::string undealt_with;
    };

    FileWatcherObj* InitTailFileWatcher(const std::string& folder_path);
    bool TailFileWatcherDealWith(FileWatcherObj* obj, const std::string& folder_path);
}