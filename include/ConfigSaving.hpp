#pragma once
#include <QString>
#include <string>

namespace ConfigSaving {
    struct ConfigData {
        std::string installDir = "";
    };

    extern ConfigData Current;

    void Load();
    void Save();
    QString GetConfigPath();
}