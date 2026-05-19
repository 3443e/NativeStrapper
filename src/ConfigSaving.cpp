#include "ConfigSaving.hpp"
#include "ScatterManager.hpp"
#include "json.hpp"
#include <fstream>
#include <sstream>
#include <QStandardPaths>
#include <QDir>
#include <QString>

using json = nlohmann::json;

static QString getDataDir() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(dir);
    return dir;
}

void ConfigSaving::SaveScatters() {
    json scatters = json::array();

    for (const auto &scatter : ScatterManager::LoadedScatters) {
        json s;
        s["ScatterTitle"] = scatter.ScatterTitle;
        s["RobloxURI"] = scatter.RobloxURI;
        s["RobloxRunCommand"] = scatter.RobloxRunCommand;
        s["RobloxAppDataDirectory"] = scatter.RobloxAppDataDirectory;
        s["RobloxURIs"] = scatter.RobloxURIs;
        scatters.push_back(s);
    }

    std::ofstream file((getDataDir() + "/scatters.json").toStdString());
    file << scatters.dump(4);
}

void ConfigSaving::LoadScatters() {
    std::ifstream file((getDataDir() + "/scatters.json").toStdString());
    if (!file.is_open()) return;

    json j;
    try {
        file >> j;
    } catch (...) {
        return;
    }

    if (!j.is_array()) return;

    ScatterManager::LoadedScatters.clear();
    for (const auto &s : j) {
        ScatterManager::Scatter scatter;
        scatter.ScatterTitle = s.value("ScatterTitle", "");
        scatter.RobloxURI = s.value("RobloxURI", "");
        scatter.RobloxRunCommand = s.value("RobloxRunCommand", "");
        scatter.RobloxAppDataDirectory = s.value("RobloxAppDataDirectory", "");
        if (s.contains("RobloxURIs") && s["RobloxURIs"].is_array()) {
            for (const auto &uri : s["RobloxURIs"]) {
                scatter.RobloxURIs.push_back(uri.get<std::string>());
            }
        }
        ScatterManager::LoadedScatters.push_back(scatter);
    }
}