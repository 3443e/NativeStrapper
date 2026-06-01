#include <thread>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <dirent.h>
#include "DiscordRPC/ActivityWatcher.hpp"
#include "DiscordRPC/TailFileWatcher.hpp"
#include "DiscordRPC/DiscordRPC.hpp"
#include "Logger.hpp"

// You can probably get away with including this thing directly but i'm a morron
extern "C" {
#include "RoLogParser.h"
}

static const std::string LOG_DIR = "%LOCALAPPDATA%\Roblox\logs";

void MainActivityWatcher() {
    auto* watcher = TailFileWatcher::InitTailFileWatcher(LOG_DIR);
    RoLogObject* currentlogobj = nullptr;
    bool wasingame = false;

    DiscordRPC::InitDiscordRPC();
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        TailFileWatcher::TailFileWatcherDealWith(watcher, LOG_DIR);

        std::stringstream ss(watcher->undealt_with);
        std::string line;
        while (std::getline(ss, line)) {
            if (!currentlogobj && !watcher->files.empty()) {
                currentlogobj = RoLogCreateObject(nullptr);
            }
            if (!currentlogobj) {
                continue;
            }
            RoLogParseLine(currentlogobj, line.c_str());
        }
        watcher->undealt_with.clear();

        if (!currentlogobj) {
            continue;
        }

        DiscordRPC::RunCallbacks();
        if (currentlogobj->current_state == ROLOG_IN_GAME || currentlogobj->current_state == ROLOG_IN_LUA_APP) {
            if (!wasingame) {
                Logger::Log("Player joined game!! placeId: " + std::to_string(currentlogobj->placeId) + " / userId: " + std::to_string(currentlogobj->userId) + " / universeId: " + std::to_string(currentlogobj->universeId), Logger::SLOG, "MainActivityWatcher");
                wasingame = true;
                DiscordRPC::SetDiscordPresence(currentlogobj->placeId, currentlogobj->universeId);
            }
        } else {
            if (wasingame) {
                Logger::Log("Player has left the game...", Logger::SLOG, "MainActivityWatcher");
                DiscordRPC::ClearDiscordPresence();
                DiscordRPC::ShutdownDiscordRPC();
                exit(0);
            }
        }
    }
}

void ActivityWatcher::StartWatcherThread() {
    std::thread watcherthread(MainActivityWatcher);
    
    {
        std::stringstream ss;
        ss << "Started activity watcher with thread ID: " << watcherthread.get_id();

        Logger::Log(ss.str(), Logger::SINFO, "StartWatcherThread");
    }

    watcherthread.detach();
    Logger::Log("Detached activity watcher thread..", Logger::SLOG, "StartWatcherThread");
}