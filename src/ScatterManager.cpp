#include <qdebug.h>
#include <sstream>
#include <QSettings>
#include <QProcess>
#include <QDir>
#include <QCoreApplication>
#include "json.hpp"
#include "ScatterManager.hpp"
#include "ConfigSaving.hpp"

using json = nlohmann::json;

namespace ScatterManager {
    std::vector<Scatter> LoadedScatters;
}

ScatterManager::Scatter* ScatterManager::ParseScatter(std::string data) {
    json j = json::parse(data);

    ScatterManager::Scatter* scatter = new ScatterManager::Scatter;
    std::string urisRaw = j.value("RobloxURIs", "");
    std::stringstream ss(urisRaw);
    std::string token;
    while (std::getline(ss, token, ',')) {
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        scatter->RobloxURIs.push_back(token);
    }

    scatter->RobloxAppDataDirectory = j.value("RobloxAppDataDirectory", "");
    scatter->RobloxRunCommand = j.value("RobloxRunCommand", "");
    scatter->ScatterTitle = j.value("ScatterTitle", "");
    
    return scatter;
}

ScatterManager::ScatterInstallResult ScatterManager::InstallScatter(ScatterManager::Scatter* scatter) {
    ScatterInstallResult result = ScatterInstallResult::SINSTALLUNKNOWNENVIRONMENT;

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