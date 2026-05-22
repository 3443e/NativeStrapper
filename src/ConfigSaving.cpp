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

        json required = json::array();
        for (const auto &r : scatter.Required) {
            required.push_back(r);
        }
        s["Required"] = required;

        json platform = json::array();
        for (const auto &p : scatter.Platform) {
            platform.push_back(p);
        }
        s["Platform"] = platform;

        if (scatter.HasBootstrap) {
            json bootstrap;

            json initRequests = json::array();
            for (const auto &r : scatter.Bootstrap.InitRequests) {
                json entry;
                entry["url"] = r.url;
                entry["token"] = r.token;
                json headers;
                for (const auto &[key, val] : r.headers) {
                    headers[key] = val;
                }
                entry["headers"] = headers;
                initRequests.push_back(entry);
            }
            bootstrap["InitRequests"] = initRequests;


            json initCommands = json::array();
            for (const auto &c : scatter.Bootstrap.InitCommands) {
                json entry;
                entry["system"] = c.system;
                entry["token"] = c.token;
                initCommands.push_back(entry);
            }
            bootstrap["InitCommands"] = initCommands;

            json preDownloadRequests = json::array();
            for (const auto &r : scatter.Bootstrap.PreDownloadRequests) {
                json entry;
                entry["url"] = r.url;
                entry["token"] = r.token;
                json headers;
                for (const auto &[key, val] : r.headers) {
                    headers[key] = val;
                }
                entry["headers"] = headers;
                preDownloadRequests.push_back(entry);
            }
            bootstrap["PreDownloadRequests"] = preDownloadRequests;


            json preDownloadCommands = json::array();
            for (const auto &c : scatter.Bootstrap.PreDownloadCommands) {
                json entry;
                entry["system"] = c.system;
                entry["token"] = c.token;
                preDownloadCommands.push_back(entry);
            }
            bootstrap["PreDownloadCommands"] = preDownloadCommands;

            json currentVersion = json::array();
            for (const auto &e : scatter.Bootstrap.CurrentVersion) {
                json entry;
                entry["system"] = e.system;
                entry["token"] = e.token;
                entry["file"] = e.file;
                currentVersion.push_back(entry);
            }
            bootstrap["CurrentVersion"] = currentVersion;

            json latestVersion = json::array();
            for (const auto &e : scatter.Bootstrap.LatestVersion) {
                json entry;
                entry["url"] = e.url;
                entry["system"] = e.system;
                entry["token"] = e.token;
                entry["file"] = e.file;
                latestVersion.push_back(entry);
            }
            bootstrap["LatestVersion"] = latestVersion;

            json download = json::array();
            for (const auto &e : scatter.Bootstrap.Download) {
                json entry;
                entry["url"] = e.url;
                entry["out"] = e.out;
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

        if (s.contains("Required") && s["Required"].is_array()) {
            for (const auto &r : s["Required"]) {
                scatter.Required.push_back(r.get<std::string>());
            }   
        }

        if (s.contains("Platform") && s["Platform"].is_array()) {
            for (const auto &p : s["Platform"]) {
                scatter.Platform.push_back(p.get<std::string>());
            }
        }
                
        if (s.contains("BootstrapDownload")) {
            scatter.HasBootstrap = true;
            auto &b = s["BootstrapDownload"];

            if (b.contains("InitRequests") && b["InitRequests"].is_array()) {
                for (const auto &entry : b["InitRequests"]) {
                    ScatterManager::BootstrapInitRequest r;
                    r.url = entry.value("url", "");
                    r.token = entry.value("token", "");
                    if (entry.contains("headers") && entry["headers"].is_object()) {
                        for (const auto &[key, val] : entry["headers"].items()) {
                            r.headers[key] = val.get<std::string>();
                        }
                    }
                    scatter.Bootstrap.InitRequests.push_back(r);
                }
            }

            if (b.contains("InitCommands") && b["InitCommands"].is_array()) {
                for (const auto &entry : b["InitCommands"]) {
                    ScatterManager::BootstrapInitCommand c;
                    c.system = entry.value("system", "");
                    c.token = entry.value("token", "");
                    scatter.Bootstrap.InitCommands.push_back(c);
                }
            }

            if (b.contains("PreDownloadRequests") && b["PreDownloadRequests"].is_array()) {
                for (const auto &entry : b["PreDownloadRequests"]) {
                    ScatterManager::BootstrapInitRequest r;
                    r.url = entry.value("url", "");
                    r.token = entry.value("token", "");
                    if (entry.contains("headers") && entry["headers"].is_object()) {
                        for (const auto &[key, val] : entry["headers"].items()) {
                            r.headers[key] = val.get<std::string>();
                        }
                    }
                    scatter.Bootstrap.PreDownloadRequests.push_back(r);
                }
            }

            if (b.contains("PreDownloadCommands") && b["PreDownloadCommands"].is_array()) {
                for (const auto &entry : b["PreDownloadCommands"]) {
                    ScatterManager::BootstrapInitCommand c;
                    c.system = entry.value("system", "");
                    c.token = entry.value("token", "");
                    scatter.Bootstrap.PreDownloadCommands.push_back(c);
                }
            }

            if (b.contains("CurrentVersion") && b["CurrentVersion"].is_array()) {
                for (const auto &entry : b["CurrentVersion"]) {
                    ScatterManager::BootstrapVersionEntry e;
                    e.system = entry.value("system", "");
                    e.token = entry.value("token", "");
                    e.file = entry.value("file", "");
                    scatter.Bootstrap.CurrentVersion.push_back(e);
                }
            }

            if (b.contains("LatestVersion") && b["LatestVersion"].is_array()) {
                for (const auto &entry : b["LatestVersion"]) {
                    ScatterManager::BootstrapVersionEntry e;
                    e.url = entry.value("url", "");
                    e.system = entry.value("system", "");
                    e.file = entry.value("file", "");
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
                    e.out = entry.value("out", "");
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