#include "ConfigSaving.hpp"
#include "VesselManager.hpp"
#include "json.hpp"
#include <fstream>
#include <QStandardPaths>
#include <QDir>
#include <QString>

using json = nlohmann::json;

static QString getDataDir() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(dir);
    return dir;
}

void ConfigSaving::SaveVessels() {
    json vessels = json::array();

    for (const auto &vessel : VesselManager::LoadedVessels) {
        json s;
        s["VesselTitle"] = vessel.VesselTitle;
        s["RobloxURI"] = vessel.RobloxURI;
        s["RobloxRunCommand"] = vessel.RobloxRunCommand;
        json dirs = json::array();
        for (const auto &dir : vessel.AppDataDirectories) {
            json d;
            d["path"]  = dir.path;
            d["label"] = dir.label;
            dirs.push_back(d);
        }
        s["AppDataDirectories"] = dirs;
        s["RobloxURIs"] = vessel.RobloxURIs;
        vessels.push_back(s);
    }

    std::ofstream file((getDataDir() + "/vessels.json").toStdString());
    file << vessels.dump(4);
}

void ConfigSaving::LoadVessels() {
    std::ifstream file((getDataDir() + "/vessels.json").toStdString());
    if (!file.is_open()) return;

    json j;
    try {
        file >> j;
    } catch (...) {
        return;
    }

    if (!j.is_array()) return;

    VesselManager::LoadedVessels.clear();
    for (const auto &s : j) {
        VesselManager::Vessel vessel;
        vessel.VesselTitle = s.value("VesselTitle", "");
        vessel.RobloxURI = s.value("RobloxURI", "");
        vessel.RobloxRunCommand = s.value("RobloxRunCommand", "");
        if (s.contains("AppDataDirectories") && s["AppDataDirectories"].is_array()) {
            for (const auto &entry : s["AppDataDirectories"]) {
                VesselManager::AppDataDirectory dir;
                dir.path = entry.value("path", "");
                dir.label = entry.value("label", "");
                vessel.AppDataDirectories.push_back(dir);
            }
        }
        if (s.contains("RobloxURIs") && s["RobloxURIs"].is_array()) {
            for (const auto &uri : s["RobloxURIs"]) {
                vessel.RobloxURIs.push_back(uri.get<std::string>());
            }
        }
        VesselManager::LoadedVessels.push_back(vessel);
    }
}