// gmmmmmmmmmmmmmmmmmmmmmmdelgrgtrrtgdoe code

#include <filesystem>
#include <string>

#include <fstream>
#include <unordered_map>
#include "DiscordRPC/TailFileWatcher.hpp"

TailFileWatcher::FileWatcherObj* TailFileWatcher::InitTailFileWatcher(const std::vector<std::string>& folders) {
    auto* obj = new TailFileWatcher::FileWatcherObj();

    for (const auto& folder_path : folders) {
        std::filesystem::path latestPath;
        std::filesystem::file_time_type latestTime;
        bool foundAny = false;

        for (const auto& entry : std::filesystem::directory_iterator(folder_path)) {
            if (!entry.is_regular_file()) continue;

            auto t = entry.last_write_time();

            if (!foundAny || t > latestTime) {
                latestTime = t;
                latestPath = entry.path();
                foundAny = true;
            }
        }

        if (foundAny) {
            TailFileWatcher::FileInfo info;
            info.path = latestPath;
            info.last_pos = std::filesystem::file_size(latestPath);

            obj->filesByFolder[folder_path][latestPath.string()] = info;
        }
    }

    return obj;
}

bool TailFileWatcher::TailFileWatcherDealWith(TailFileWatcher::FileWatcherObj* obj, const std::vector<std::string>& folders) {
    if (!obj) return false;
    obj->undealt_with.clear();
    for (const auto& folder_path : folders) {
        auto& files = obj->filesByFolder[folder_path];

        for (const auto& entry : std::filesystem::directory_iterator(folder_path)) {
            if (!entry.is_regular_file()) continue;
            auto pathStr = entry.path().string();

            if (files.find(pathStr) == files.end()) {
                std::filesystem::file_time_type newTime;

                try {
                    newTime = entry.last_write_time();
                } catch (...) {
                    continue;
                }

                bool isNewer = true;
                for (auto& [p, f] : files) {
                    try {
                        if (std::filesystem::last_write_time(f.path) >= newTime) {
                            isNewer = false;
                            break;
                        }
                    } catch (...) {}
                }

                if (isNewer) {
                    files.clear();

                    TailFileWatcher::FileInfo info;
                    info.path = entry.path();
                    info.last_pos = 0;

                    files[pathStr] = info;
                }
            }
        }

        for (auto& [pathStr, file] : files)
        {
            std::ifstream in(file.path, std::ios::binary);
            if (!in) continue;

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
    }

    return true;
}