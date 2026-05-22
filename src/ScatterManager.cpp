#include <qdebug.h>
#include <sstream>
#include <QSettings>
#include <QProcess>
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>
#include "json.hpp"
#include "ScatterManager.hpp"
#include "ConfigSaving.hpp"

using json = nlohmann::json;

namespace ScatterManager {
    std::vector<Scatter> LoadedScatters;
}

static std::string expandTokens(const std::string &str) {
    std::string result = str;

    // %home / %user token replacement
#ifdef _WIN32
    const char *user = getenv("USERNAME");
    const char *home = getenv("USERPROFILE");
#else
    const char *user = getenv("USER");
    if (!user) user = getenv("LOGNAME");
    const char *home = getenv("HOME");
#endif

    auto replace = [&](const std::string &token, const char *value) {
        if (!value) return;
        size_t pos;
        while ((pos = result.find(token)) != std::string::npos) {
            // make sure it's not part of a longer token like %username
            size_t after = pos + token.size();
            if (after < result.size() && (isalnum(result[after]) || result[after] == '_')) {
                break;
            } 
            result.replace(pos, token.size(), value);
        }
    };

    replace("%user", user);
    replace("%home", home);

    return result;
}

ScatterManager::Scatter* ScatterManager::ParseScatter(std::string data) {
try { /* indentation evil */
    json j = json::parse(data);
    ScatterManager::Scatter* scatter = new ScatterManager::Scatter;

    if (j.contains("RobloxURIs")) {
        if (j["RobloxURIs"].is_array()) {
            for (const auto &uri : j["RobloxURIs"]) {
                scatter->RobloxURIs.push_back(uri.get<std::string>());
            }
        } else if (j["RobloxURIs"].is_string()) {  /* comma separated */
            std::string urisRaw = j["RobloxURIs"].get<std::string>();
            std::stringstream ss(urisRaw);
            std::string token;
            while (std::getline(ss, token, ',')) {
                token.erase(0, token.find_first_not_of(" \t"));
                token.erase(token.find_last_not_of(" \t") + 1);
                scatter->RobloxURIs.push_back(token);
            }
        }
    }

    scatter->RobloxRunCommand = expandTokens(j.value("RobloxRunCommand", ""));
    scatter->ScatterTitle = j.value("ScatterTitle", "");
    
    if (j.contains("AppDataDirectories") && j["AppDataDirectories"].is_array()) {
        for (const auto &entry : j["AppDataDirectories"]) {
            ScatterManager::AppDataDirectory dir;
            dir.path = expandTokens(entry.value("path", ""));
            dir.label = expandTokens(entry.value("label", ""));
            scatter->AppDataDirectories.push_back(dir);
        }
    }

    if (j.contains("Required") && j["Required"].is_array()) {
        for (const auto &req : j["Required"]) {
            scatter->Required.push_back(req.get<std::string>());
        }
    }

    if (j.contains("Platform")) {
        if (j["Platform"].is_array()) {
            for (const auto &p : j["Platform"]) {
                scatter->Platform.push_back(p.get<std::string>());
            }
        } else if (j["Platform"].is_string()) {
            scatter->Platform.push_back(j["Platform"].get<std::string>());
        }
    }


    if (j.contains("BootstrapDownload")) {
        auto &b = j["BootstrapDownload"];
        scatter->HasBootstrap = true;

        // parse InitRequests
        if (b.contains("InitRequests") && b["InitRequests"].is_array()) {
            for (const auto &entry : b["InitRequests"]) {
                BootstrapInitRequest r;
                r.url = entry.value("url", "");
                r.token = entry.value("token", "");
                if (entry.contains("headers") && entry["headers"].is_object()) {
                    for (const auto &[key, val] : entry["headers"].items()) {
                        r.headers[key] = val.get<std::string>();
                    }
                }
                scatter->Bootstrap.InitRequests.push_back(r);
            }
        }

        // parse InitCommands
        if (b.contains("InitCommands") && b["InitCommands"].is_array()) {
            for (const auto &entry : b["InitCommands"]) {
                BootstrapInitCommand c;
                c.system = entry.value("system", "");
                c.token = entry.value("token", "");
                scatter->Bootstrap.InitCommands.push_back(c);
            }
        }

        // parse PreDownloadRequests
        if (b.contains("PreDownloadRequests") && b["PreDownloadRequests"].is_array()) {
            for (const auto &entry : b["PreDownloadRequests"]) {
                BootstrapInitRequest r;
                r.url = entry.value("url", "");
                r.token = entry.value("token", "");
                if (entry.contains("headers") && entry["headers"].is_object()) {
                    for (const auto &[key, val] : entry["headers"].items()) {
                        r.headers[key] = val.get<std::string>();
                    }
                }
                scatter->Bootstrap.PreDownloadRequests.push_back(r);
            }
        }

        // predownloadcommands
        if (b.contains("PreDownloadCommands") && b["PreDownloadCommands"].is_array()) {
            for (const auto &entry : b["PreDownloadCommands"]) {
                BootstrapInitCommand c;
                c.system = entry.value("system", "");
                c.token = entry.value("token", "");
                scatter->Bootstrap.PreDownloadCommands.push_back(c);
            }
        }
        // parse CurrentVersion
        if (b.contains("CurrentVersion") && b["CurrentVersion"].is_array()) {
            for (const auto &entry : b["CurrentVersion"]) {
                BootstrapVersionEntry e;
                e.system = expandTokens(entry.value("system", ""));
                e.token = entry.value("token", "");
                e.file = expandTokens(entry.value("file", ""));
                scatter->Bootstrap.CurrentVersion.push_back(e);
            }
        }

        // parse LatestVersion
        if (b.contains("LatestVersion") && b["LatestVersion"].is_array()) {
            for (const auto &entry : b["LatestVersion"]) {
                BootstrapVersionEntry e;
                e.url = expandTokens(entry.value("url", ""));
                e.system = expandTokens(entry.value("system", ""));
                e.file = expandTokens(entry.value("file", ""));
                e.token = entry.value("token", "");
                scatter->Bootstrap.LatestVersion.push_back(e);
            }
        }

        for (const auto &e : scatter->Bootstrap.LatestVersion) {
            if (e.token.empty() && e.file.empty()) {
                scatter->HasBootstrap = false;
                scatter->BootstrapError.error = BootstrapParseError::BPARSE_INVALID_LATEST_VERSION;
                scatter->BootstrapError.message = "A LatestVersion entry must have either a token or a file field.";
            }
        }

        for (const auto &e : scatter->Bootstrap.CurrentVersion) {
            if (e.token.empty() && e.file.empty()) {
                scatter->HasBootstrap = false;
                scatter->BootstrapError.error = BootstrapParseError::BPARSE_INVALID_LATEST_VERSION;
                scatter->BootstrapError.message = "A CurrentVersion entry must have either a token or a file field.";
            }
        }

        // needs to have same amount as fields/entires/arrays or whatever
        if (scatter->Bootstrap.CurrentVersion.size() != scatter->Bootstrap.LatestVersion.size()) {
            scatter->HasBootstrap = false;
            scatter->BootstrapError.error = BootstrapParseError::BPARSE_VERSION_COUNT_MISMATCH;

            auto currentSize = scatter->Bootstrap.CurrentVersion.size();
            auto latestSize = scatter->Bootstrap.LatestVersion.size();

            auto currentWord = (currentSize == 1) ? "entry" : "entries";
            auto latestWord  = (latestSize == 1) ? "entry" : "entries";

            scatter->BootstrapError.message = "CurrentVersion has " + std::to_string(currentSize) + " " + currentWord + " but LatestVersion has " + std::to_string(latestSize) + " " + latestWord + ", they must match.";
        }

        for (const auto &e : scatter->Bootstrap.CurrentVersion) {
            if (e.token.empty()) {
                scatter->HasBootstrap = false;
                scatter->BootstrapError.error = BootstrapParseError::BPARSE_MISSING_TOKEN;
                scatter->BootstrapError.message = "A CurrentVersion entry is missing a token.";
            }
            if (e.system.empty()) {
                scatter->HasBootstrap = false;
                scatter->BootstrapError.error = BootstrapParseError::BPARSE_INVALID_CURRENT_VERSION;
                scatter->BootstrapError.message = "A CurrentVersion entry is missing a system command.";
            }
        }
        
        for (const auto &e : scatter->Bootstrap.LatestVersion) {
            if (e.token.empty()) {
                scatter->HasBootstrap = false;
                scatter->BootstrapError.error = BootstrapParseError::BPARSE_MISSING_TOKEN;
                scatter->BootstrapError.message = "A LatestVersion entry is missing a token.";
            }
            if (e.url.empty() && e.system.empty()) {
                scatter->HasBootstrap = false;
                scatter->BootstrapError.error = BootstrapParseError::BPARSE_INVALID_LATEST_VERSION;
                scatter->BootstrapError.message = "A LatestVersion entry must have either a url or a system command.";
            }
        }

        // parse Download
        if (b.contains("Download") && b["Download"].is_array()) {
            for (const auto &entry : b["Download"]) {
                BootstrapDownloadEntry e;
                e.url = expandTokens(entry.value("url", ""));
                e.out = expandTokens(entry.value("out", ""));
                scatter->Bootstrap.Download.push_back(e);
            }
        }

        // parse AfterDownload
        if (b.contains("AfterDownload") && b["AfterDownload"].is_array()) {
            for (const auto &cmd : b["AfterDownload"]) {
                scatter->Bootstrap.AfterDownload.push_back(cmd.get<std::string>());
            }
        }
    }
    return scatter;
} catch (const std::exception &e) {
    return nullptr;
}
}

ScatterManager::ScatterInstallResult ScatterManager::InstallScatter(ScatterManager::Scatter* scatter) {
    ScatterInstallResult result = ScatterInstallResult::SINSTALLUNKNOWNENVIRONMENT;

    if (!scatter->Platform.empty()) {
        std::string current;
#ifdef __linux__
        current = "Linux";
#elif _WIN32
        current = "Windows";
#elif __ANDROID__
        current = "Android";
#elif __APPLE__
        current = "macOS";
#endif
        bool supported = false;
        for (const auto &p : scatter->Platform) {
            if (p == current) {
                supported = true;
                break;
            }
        }
        if (!supported) return ScatterInstallResult::SINSTALLUNSUPPORTEDPLATFORM;
    }

    for (const auto &cmd : scatter->Required) {
        if (QStandardPaths::findExecutable(QString::fromStdString(cmd)).isEmpty()) {
            return ScatterInstallResult::SINSTALLMISSINGREQUIRED;
        }
    }

#ifdef __linux__
    QString title = QString::fromStdString(scatter->ScatterTitle);
    QString safeTitle = title.toLower().replace(" ", "-");
    QString desktopPath = QDir::homePath() + "/.local/share/applications/nativestrapper-" + safeTitle + ".desktop";
    QString execPath = QCoreApplication::applicationFilePath();

    // build MimeType from RobloxURIs vector
    QString mimeType;
    for (const auto &uri : scatter->RobloxURIs) {
        mimeType += "x-scheme-handler/" + QString::fromStdString(uri).trimmed() + ";";
    }

    QString content =
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=" + title + "\n"
        "Exec=" + execPath + " --scatter " + safeTitle + " %u\n"
        "MimeType=" + mimeType + "\n"
        "Terminal=false\n"
        "NoDisplay=true\n";

    QFile file(desktopPath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << content;
        file.close();
    }

    // register each URI handler
    for (const auto &uri : scatter->RobloxURIs) {
        QString scheme = "x-scheme-handler/" + QString::fromStdString(uri).trimmed();
        QProcess::execute("xdg-mime", {"default", "nativestrapper-" + safeTitle + ".desktop", scheme});
    }
    QProcess::execute("update-desktop-database", {QDir::homePath() + "/.local/share/applications/"});
    result = ScatterManager::ScatterInstallResult::SINSTALLSUCCESS;
#endif

    if (result == ScatterInstallResult::SINSTALLSUCCESS) {
        LoadedScatters.push_back(*scatter);
        ConfigSaving::SaveScatters();
    }

    return result;
}

ScatterManager::ScatterUninstallResult ScatterManager::UninstallScatter(ScatterManager::Scatter* scatter) {
    ScatterUninstallResult result = ScatterUninstallResult::SUNINSTALLUNKNOWNENVIRONMENT;
#ifdef __linux__
    QString safeTitle = QString::fromStdString(scatter->ScatterTitle).toLower().replace(" ", "-");
    QString desktopPath = QDir::homePath() + "/.local/share/applications/nativestrapper-" + safeTitle + ".desktop";

    QString mimeappsPath = QDir::homePath() + "/.config/mimeapps.list";
    QSettings mimeapps(mimeappsPath, QSettings::IniFormat);
    mimeapps.beginGroup("Default Applications");
    for (const auto &uri : scatter->RobloxURIs) {
        mimeapps.remove("x-scheme-handler/" + QString::fromStdString(uri).trimmed());
    }
    mimeapps.endGroup();

    // delete the .desktop file
    QFile file(desktopPath);
    if (file.exists()) {
        if (!file.remove()) {
            result = ScatterManager::ScatterUninstallResult::SUNINSTALLFAILED;
        }
    }

    QProcess::execute("update-desktop-database", {QDir::homePath() + "/.local/share/applications/"});

    result = ScatterManager::ScatterUninstallResult::SUNINSTALLSUCCESS;
#endif

    if (result == ScatterUninstallResult::SUNINSTALLSUCCESS) {
        LoadedScatters.erase(std::remove_if(LoadedScatters.begin(), LoadedScatters.end(),
            [&](const Scatter &s) { return s.ScatterTitle == scatter->ScatterTitle; }
        ), LoadedScatters.end());
        ConfigSaving::SaveScatters();
    }

    return result;
}