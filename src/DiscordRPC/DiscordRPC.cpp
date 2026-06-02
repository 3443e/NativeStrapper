#include "DiscordRPC/DiscordRPC.hpp"
#include "NetworkManagement.hpp"
#include "Logger.hpp"
#include "discord-rpc.hpp"
#include "json.hpp"
#include <ctime>
#include <string>

using json = nlohmann::json;

static std::string FetchGameInfo(uint64_t universeId, const std::string& field) {
    std::string url = "https://games.roblox.com/v1/games?universeIds=" + std::to_string(universeId);
    std::string body = NetworkManagement::FetchURL(url, {});
    if (body.empty()) {
        return "";
    }
    try {
        auto j = json::parse(body);
        if (field == "creator") {
            return j["data"][0]["creator"]["name"].get<std::string>();
        }
        return j["data"][0][field].get<std::string>();
    } catch (...) {
        return "";
    }
}

static std::string FetchThumbnailUrl(uint64_t universeId) {
    std::string url = "https://thumbnails.roblox.com/v1/games/icons?universeIds=" + std::to_string(universeId) + "&size=512x512&format=Png&isCircular=false";
    std::string body = NetworkManagement::FetchURL(url, {});
    if (body.empty()) return "";
    try {
        auto j = json::parse(body);
        return j["data"][0]["imageUrl"].get<std::string>();
    } catch (...) {
        return "";
    }
}

void DiscordRPC::InitDiscordRPC() {
    auto& rpc = discord::RPCManager::get();
    rpc.setClientID("1511131982355238962")
       .onReady([](discord::User const& user) {
           Logger::Log("Discord RPC ready: " + user.username, Logger::SLOG, "DiscordRPC");
       })
       .onDisconnected([](int code, std::string_view msg) {
           Logger::Log("Discord RPC disconnected: " + std::string(msg), Logger::SLOG, "DiscordRPC");
       })
       .onErrored([](int code, std::string_view msg) {
           Logger::Log("Discord RPC error: " + std::string(msg), Logger::SLOG, "DiscordRPC");
       })
       .initialize();

    Logger::Log("Discord RPC initialized", Logger::SLOG, "DiscordRPC");
}

void DiscordRPC::SetDiscordPresence(uint64_t placeId, uint64_t universeId) {
    Logger::Log("Fetching game info for universeId: " + std::to_string(universeId), Logger::SLOG, "SetDiscordPresence");

    std::string gameName = FetchGameInfo(universeId, "name");
    std::string creatorName = FetchGameInfo(universeId, "creator");
    std::string thumbnail = FetchThumbnailUrl(universeId);
    std::string gamePageUrl = "https://www.roblox.com/games/" + std::to_string(placeId);
    std::string stateStr = "by " + (creatorName.empty() ? "Unknown" : creatorName);
    if (gameName.empty()) {
        gameName = "Roblox";
    }

    Logger::Log("Setting presence: " + gameName + " / " + stateStr, Logger::SLOG, "SetDiscordPresence");

    auto& rpc = discord::RPCManager::get();
    rpc.getPresence()
       .setDetails(gameName)
       .setState(stateStr)
       .setLargeImageKey(thumbnail)
       .setLargeImageText(gameName)
       .setStartTimestamp((int64_t)time(nullptr))
       .setButton1("See game page", gamePageUrl);

    rpc.refresh();
}

void DiscordRPC::ClearDiscordPresence() {
    discord::RPCManager::get().clearPresence();
    Logger::Log("Cleared Discord presence", Logger::SLOG, "ClearDiscordPresence");
}

void DiscordRPC::ShutdownDiscordRPC() {
    discord::RPCManager::get().shutdown();
    Logger::Log("Discord RPC shut down", Logger::SLOG, "ShutdownDiscordRPC");
}

void DiscordRPC::RunCallbacks() {
    discord::RPCManager::get().update();
}