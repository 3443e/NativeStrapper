// platform_default_handler.h
#pragma once
#include <string>

namespace URIHandler {
    // scheme e.g. "roblox-player". Returns true on success.
    bool SetAsDefaultHandler(const std::string& scheme);
    // Path of the current handler app, or "" if none.
    std::string currentDefaultHandler(const std::string& scheme);
}