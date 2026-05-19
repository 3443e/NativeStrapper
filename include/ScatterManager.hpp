#pragma once

#include <string>
#include <vector>

namespace ScatterManager {
    struct Scatter {
        bool Active = true;
        std::string RobloxURI;
        std::string RobloxRunCommand;
        std::string RobloxAppDataDirectory;
        std::string ScatterTitle;
    };

    extern std::vector<Scatter> LoadedScatters;

    // this function returns a heap allocated scatter struct, give it a scatter json string.
    Scatter* ParseScatter(std::string);
}
