// gmmmmmmmmmmmmmmmmmmmmmmdelgrgtrrtgdoe code

#include <filesystem>
#include <string>
#include <fstream>
#include <unordered_map>
#include "DiscordRPC/TailFileWatcher.hpp"

TailFileWatcher::FileWatcherObj* TailFileWatcher::InitTailFileWatcher(
    const std::string& folder_path) {
    auto* obj = new TailFileWatcher::FileWatcherObj();

    std::filesystem::path latestPath;
    std::filesystem::file_time_type latestTime;

    for (const auto& entry : std::filesystem::directory_iterator(folder_path)) {
        if (!entry.is_regular_file()) continue;

        auto t = entry.last_write_time();
        if (latestPath.empty() || t > latestTime) {
            latestTime = t;
            latestPath = entry.path();
        }
    }

    if (!latestPath.empty()) {
        TailFileWatcher::FileInfo info;
        info.path = latestPath;
        info.last_pos = std::filesystem::file_size(latestPath);
        obj->files[latestPath.string()] = info;
    }

    return obj;
}

bool TailFileWatcher::TailFileWatcherDealWith(
    TailFileWatcher::FileWatcherObj* obj,
    const std::string& folder_path) {

    if (!obj) {
        return false;
    }

    obj->undealt_with.clear();

    for (const auto& entry : std::filesystem::directory_iterator(folder_path)) {
        if (!entry.is_regular_file()) continue;

        auto pathStr = entry.path().string();
        if (obj->files.find(pathStr) == obj->files.end()) {
            std::filesystem::file_time_type newTime;
            try {
                newTime = entry.last_write_time();
            } catch (const std::filesystem::filesystem_error&) {
                continue;
            }

            bool isNewer = true;
            for (auto& [p, f] : obj->files) {
                try {
                    if (std::filesystem::last_write_time(f.path) >= newTime) {
                        isNewer = false;
                        break;
                    }
                } catch (const std::filesystem::filesystem_error&) {
                   // idk what to do
                }
            }

            if (isNewer) {
                obj->files.clear();
                TailFileWatcher::FileInfo info;
                info.path = entry.path();
                info.last_pos = 0;
                obj->files[pathStr] = info;
            }
        }
    }

    for (auto& [pathStr, file] : obj->files) {
        std::ifstream in(file.path, std::ios::binary);
        if (!in) {
            continue;
        }

        in.seekg(file.last_pos);
        std::string line;
        while (std::getline(in, line)) {
            obj->undealt_with += line;
            obj->undealt_with += '\n';
        }
        
        in.clear();
        in.seekg(0, std::ios::end);
        auto endPos = in.tellg();
        if (endPos >= 0) {
            file.last_pos = (std::uintmax_t)endPos;
        }
    }

    return true;
}