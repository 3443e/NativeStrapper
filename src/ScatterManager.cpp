#include "ScatterManager.hpp"
#include "json.hpp"

using json = nlohmann::json;

namespace ScatterManager {
    std::vector<Scatter> LoadedScatters;
}

ScatterManager::Scatter* ScatterManager::ParseScatter(std::string data) {
    json j = json::parse(data);

    ScatterManager::Scatter* scatter = new ScatterManager::Scatter;
    scatter->RobloxURI = j.value("RobloxURI", "");
    scatter->RobloxAppDataDirectory = j.value("RobloxAppDataDirectory", "");
    scatter->RobloxRunCommand = j.value("RobloxRunCommand", "");
    scatter->ScatterTitle = j.value("ScatterTitle", "");
    
    return scatter;
}

