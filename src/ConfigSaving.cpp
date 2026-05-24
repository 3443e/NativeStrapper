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

static std::string getConfigPath() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(dir);
    return (dir + "/config.json").toStdString();
}

void ConfigSaving::Load() {
    std::ifstream f(getConfigPath());
    if (!f.is_open()) return;

    json j;
    try {
        f >> j;
    } catch (...) {
        return;
    }

    Current.installDir = j.value("InstallDirectory", Current.installDir);
}

void ConfigSaving::Save() {
    json j;

    j["InstallDirectory"] = Current.installDir;

    std::ofstream f(getConfigPath());
    f << j.dump(4);
}