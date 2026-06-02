#pragma once
#include <string>
#include <cstdint>

namespace DiscordRPC {
    void InitDiscordRPC();
    void SetDiscordPresence(uint64_t placeId, uint64_t universeId);
    void ClearDiscordPresence();
    void ShutdownDiscordRPC();
    void RunCallbacks();
}