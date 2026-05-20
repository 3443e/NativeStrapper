#include "ConfigSaving.hpp"
#include "ScatterManager.hpp"
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

void ConfigSaving::SaveScatters() {
    json scatters = json::array();

    for (const auto &scatter : ScatterManager::LoadedScatters) {
        json s;
        s["ScatterTitle"] = scatter.ScatterTitle;
        s["RobloxURI"] = scatter.RobloxURI;
        s["RobloxRunCommand"] = scatter.RobloxRunCommand;
        json dirs = json::array();
        for (const auto &dir : scatter.AppDataDirectories) {
            json d;
            d["path"] = dir.path;
            d["label"] = dir.label;
            dirs.push_back(d);
        }
        s["AppDataDirectories"] = dirs;
        s["RobloxURIs"] = scatter.RobloxURIs;
        if (scatter.HasBootstrap) {
            json bootstrap;

            json currentVersion = json::array();
            for (const auto &e : scatter.Bootstrap.CurrentVersion) {
                json entry;
                entry["system"] = e.system;
                entry["token"] = e.token;
                currentVersion.push_back(entry);
            }
            bootstrap["CurrentVersion"] = currentVersion;

            json latestVersion = json::array();
            for (const auto &e : scatter.Bootstrap.LatestVersion) {
                json entry;
                entry["url"] = e.url;
                entry["system"] = e.system;
                entry["token"] = e.token;
                latestVersion.push_back(entry);
            }
            bootstrap["LatestVersion"] = latestVersion;

            json download = json::array();
            for (const auto &e : scatter.Bootstrap.Download) {
                json entry;
                entry["url"] = e.url;
                entry["directory"] = e.directory;
                download.push_back(entry);
            }
            bootstrap["Download"] = download;

            json afterDownload = json::array();
            for (const auto &cmd : scatter.Bootstrap.AfterDownload) {
                afterDownload.push_back(cmd);
            }
            bootstrap["AfterDownload"] = afterDownload;

            s["BootstrapDownload"] = bootstrap;
        }
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
        if (s.contains("AppDataDirectories") && s["AppDataDirectories"].is_array()) {
            for (const auto &entry : s["AppDataDirectories"]) {
                ScatterManager::AppDataDirectory dir;
                dir.path = entry.value("path", "");
                dir.label = entry.value("label", "");
                scatter.AppDataDirectories.push_back(dir);
            }
        }
        if (s.contains("RobloxURIs") && s["RobloxURIs"].is_array()) {
            for (const auto &uri : s["RobloxURIs"]) {
                scatter.RobloxURIs.push_back(uri.get<std::string>());
            }
        }
        if (s.contains("BootstrapDownload")) {
            scatter.HasBootstrap = true;
            auto &b = s["BootstrapDownload"];

            if (b.contains("CurrentVersion") && b["CurrentVersion"].is_array()) {
                for (const auto &entry : b["CurrentVersion"]) {
                    ScatterManager::BootstrapVersionEntry e;
                    e.system = entry.value("system", "");
                    e.token = entry.value("token", "");
                    scatter.Bootstrap.CurrentVersion.push_back(e);
                }
            }

            if (b.contains("LatestVersion") && b["LatestVersion"].is_array()) {
                for (const auto &entry : b["LatestVersion"]) {
                    ScatterManager::BootstrapVersionEntry e;
                    e.url = entry.value("url", "");
                    e.system = entry.value("system", "");
                    e.token = entry.value("token", "");
                    scatter.Bootstrap.LatestVersion.push_back(e);
                }
            }

            if (scatter.Bootstrap.CurrentVersion.size() != scatter.Bootstrap.LatestVersion.size()) {
                scatter.HasBootstrap = false;
            }

            if (b.contains("Download") && b["Download"].is_array()) {
                for (const auto &entry : b["Download"]) {
                    ScatterManager::BootstrapDownloadEntry e;
                    e.url = entry.value("url", "");
                    e.directory = entry.value("directory", "");
                    scatter.Bootstrap.Download.push_back(e);
                }
            }

            if (b.contains("AfterDownload") && b["AfterDownload"].is_array()) {
                for (const auto &cmd : b["AfterDownload"]) {
                    scatter.Bootstrap.AfterDownload.push_back(cmd.get<std::string>());
                }
            }
        }
        ScatterManager::LoadedScatters.push_back(scatter);
    }
}