#include "ConfigSaving.hpp"
#include "Logger.hpp"
#include "json.hpp"
#include <fstream>
#include <QStandardPaths>
#include <QDir>

using json = nlohmann::json;

namespace ConfigSaving {
    ConfigData Current;
}

QString ConfigSaving::GetConfigPath() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(dir);
    return (dir + "/config.json");
}

void ConfigSaving::Load() {
    std::ifstream f(GetConfigPath().toStdString());
    if (!f.is_open()) {
        return;
    }

    json j;
    try { f >> j; } catch (...) { return; }

    Current.installDir = j.value("installDir", "");

    if (j.contains("scriptConfig") && j["scriptConfig"].is_object()) {
        for (auto &[scriptName, settings] : j["scriptConfig"].items()) {
            for (auto &[key, val] : settings.items()) {
                Current.scriptConfig[scriptName][key] = val.get<std::string>();
            }
        }
    }
}

void ConfigSaving::Save() {
    json j;
    j["installDir"] = Current.installDir;

    json scriptConfig;
    for (const auto &[scriptName, settings] : Current.scriptConfig) {
        for (const auto &[key, val] : settings) {
            scriptConfig[scriptName][key] = val;
        }
    }
    j["scriptConfig"] = scriptConfig;

    std::ofstream f(GetConfigPath().toStdString());
    f << j.dump(4);
}
