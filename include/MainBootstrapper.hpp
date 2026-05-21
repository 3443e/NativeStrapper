#pragma once
#include "ScatterManager.hpp"
#include <map>
#include <string>

namespace MainBootstrapper {
    enum MainStartResult {
        BOOTSTRAPSUCCESS,
        BOOTSTRAPLAUNCHERROR,
        BOOTSTRAPUPDATEERROR
    };

    using TokenMap = std::map<std::string, std::string>;
    
    MainStartResult StartStrappin(ScatterManager::Scatter*, char* URI);
}