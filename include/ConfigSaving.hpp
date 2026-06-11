#pragma once
#include <QString>
#include <string>
#include <unordered_map>

namespace ConfigSaving {
    struct ConfigData {
        std::string installDir = "";

        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> scriptConfig;
    };

    extern ConfigData Current;

    void Load();
    void Save();
    QString GetConfigPath();

    inline std::string GetScriptSetting(const std::string &scriptTitle, const std::string &key, const std::string &defaultVal = "") {
        QString safeTitle = QString::fromStdString(scriptTitle).toLower().replace(" ", "-");
        auto it = Current.scriptConfig.find(safeTitle.toStdString());
        if (it == Current.scriptConfig.end()) {
            return defaultVal;
        
        }
        auto it2 = it->second.find(key);
        if (it2 == it->second.end()) {
            return defaultVal;
        }
        return it2->second;
    }

    inline void SetScriptSetting(const std::string &scriptTitle, const std::string &key, const std::string &val) {
        QString safeTitle = QString::fromStdString(scriptTitle).toLower().replace(" ", "-");
        Current.scriptConfig[safeTitle.toStdString()][key] = val;
        //Save();
    }

    inline bool GetScriptSettingBool(const std::string &scriptTitle, const std::string &key, bool defaultVal = false) {
        std::string val = GetScriptSetting(scriptTitle, key, defaultVal ? "true" : "false");
        return val == "true";
    }
}