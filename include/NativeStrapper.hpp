#pragma once
#include <cstddef>

namespace NativeStrapper {
    struct ArgConfig {
        char* URI = NULL;
        char* BootstrapScript = NULL;
        bool ActivityWatch = false;
    };
    
    inline constexpr const char* NativeStrapperVersion = "0.10";
    inline constexpr const char* NativeStrapperRepo = "https://github.com/3443e/NativeStrapper";
    inline constexpr const char* NativeStrapperDocs = "https://nativestrapper.gitlab.io/";
    inline constexpr const char* NativeStrapperAutoUpdate = "https://raw.githubusercontent.com/3443e/NativeStrapper/refs/heads/main/auto-update.json";
    inline constexpr const char* NativeStrapperPublicBootstrapScripts = "https://raw.githubusercontent.com/3443e/NativeStrapper/refs/heads/main/providedscripts/public-scripts.json";
    inline constexpr const char* NativeStrapperDiscord = "https://discord.gg/eUSGaxZSBX";
    inline constexpr const char DiscordNativeStrapperAppID[20] = "1511131982355238962";
    inline constexpr const char DiscordRobloxAppID[20] = "1005469189907173486";

    namespace DebugConfig {
        inline constexpr unsigned int NativeStrapperLogPollMS = 1000;
    }
}
