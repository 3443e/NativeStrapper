// platform_default_handler.h
#pragma once
#include <string>

namespace platform {
    // scheme e.g. "roblox-player". Returns true on success.
    bool setAsDefaultHandler(const std::string& scheme);
    // Path of the current handler app, or "" if none.
    std::string currentDefaultHandler(const std::string& scheme);
}