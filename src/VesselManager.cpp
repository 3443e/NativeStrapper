#include <qdebug.h>
#include <sstream>
#include <QSettings>
#include <QProcess>
#include <QDir>
#include <QCoreApplication>
#include "json.hpp"
#include "VesselManager.hpp"
#include "ConfigSaving.hpp"

using json = nlohmann::json;

namespace VesselManager {
    std::vector<Vessel> LoadedVessels;
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

VesselManager::Vessel* VesselManager::ParseVessel(std::string data) {
    json j = json::parse(data);
    VesselManager::Vessel* vessel = new VesselManager::Vessel;

    std::string urisRaw = j.value("RobloxURIs", "");
    std::stringstream ss(urisRaw);
    std::string token;
    while (std::getline(ss, token, ',')) {
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        vessel->RobloxURIs.push_back(token);
    }

    vessel->RobloxRunCommand = expandTokens(j.value("RobloxRunCommand", ""));
    vessel->VesselTitle = j.value("VesselTitle", "");
    
    if (j.contains("AppDataDirectories") && j["AppDataDirectories"].is_array()) {
        for (const auto &entry : j["AppDataDirectories"]) {
            VesselManager::AppDataDirectory dir;
            dir.path  = expandTokens(entry.value("path", ""));
            dir.label = entry.value("label", "");
            vessel->AppDataDirectories.push_back(dir);
        }
    }
    return vessel;
}

VesselManager::VesselInstallResult VesselManager::InstallVessel(VesselManager::Vessel* vessel) {
    VesselInstallResult result = VesselInstallResult::VINSTALLUNKNOWNENVIRONMENT;

#ifdef __linux__
    QString title = QString::fromStdString(vessel->VesselTitle);
    QString safeTitle = title.toLower().replace(" ", "-");
    QString desktopPath = QDir::homePath() + "/.local/share/applications/nativestrapper-" + safeTitle + ".desktop";
    QString execPath = QCoreApplication::applicationFilePath();

    // build MimeType from RobloxURIs vector
    QString mimeType;
    for (const auto &uri : vessel->RobloxURIs) {
        mimeType += "x-scheme-handler/" + QString::fromStdString(uri).trimmed() + ";";
    }

    QString content =
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=" + title + "\n"
        "Exec=" + execPath + " --vessel " + safeTitle + " %u\n"
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
    for (const auto &uri : vessel->RobloxURIs) {
        QString scheme = "x-scheme-handler/" + QString::fromStdString(uri).trimmed();
        QProcess::execute("xdg-mime", {"default", "nativestrapper-" + safeTitle + ".desktop", scheme});
    }
    QProcess::execute("update-desktop-database", {QDir::homePath() + "/.local/share/applications/"});
    result = VesselManager::VesselInstallResult::VINSTALLSUCCESS;
#endif

    if (result == VesselInstallResult::VINSTALLSUCCESS) {
        LoadedVessels.push_back(*vessel);
        ConfigSaving::SaveVessels();
    }

    return result;
}

VesselManager::VesselUninstallResult VesselManager::UninstallVessel(VesselManager::Vessel* vessel) {
    VesselUninstallResult result = VesselUninstallResult::VUNINSTALLUNKNOWNENVIRONMENT;
#ifdef __linux__
    QString safeTitle = QString::fromStdString(vessel->VesselTitle).toLower().replace(" ", "-");
    QString desktopPath = QDir::homePath() + "/.local/share/applications/nativestrapper-" + safeTitle + ".desktop";

    QString mimeappsPath = QDir::homePath() + "/.config/mimeapps.list";
    QSettings mimeapps(mimeappsPath, QSettings::IniFormat);
    mimeapps.beginGroup("Default Applications");
    for (const auto &uri : vessel->RobloxURIs) {
        mimeapps.remove("x-scheme-handler/" + QString::fromStdString(uri).trimmed());
    }
    mimeapps.endGroup();

    // delete the .desktop file
    QFile file(desktopPath);
    if (file.exists()) {
        if (!file.remove()) {
            result = VesselManager::VesselUninstallResult::VUNINSTALLFAILED;
        }
    }

    QProcess::execute("update-desktop-database", {QDir::homePath() + "/.local/share/applications/"});

    result = VesselManager::VesselUninstallResult::VUNINSTALLSUCCESS;
#endif

    if (result == VesselUninstallResult::VUNINSTALLSUCCESS) {
        LoadedVessels.erase(std::remove_if(LoadedVessels.begin(), LoadedVessels.end(),
            [&](const Vessel &s) { return s.VesselTitle == vessel->VesselTitle; }
        ), LoadedVessels.end());
        ConfigSaving::SaveVessels();
    }

    return result;
}