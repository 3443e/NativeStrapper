#pragma once

#include "UserInterface/BootstrapWindow.hpp"
#include <string>

namespace LuaScript {
    // loads and runs bootstrap(uri) from the bootstrap script file
    // returns true if it ran without errors
    bool RunScript(const std::string &scriptPath, const std::string &uri, BootstrapWindow *window = nullptr);
}