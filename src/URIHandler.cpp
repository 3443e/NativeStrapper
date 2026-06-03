#include "URIHandler.hpp"
#include "URIHandlers/platform_default_handler.h"
#include "Logger.hpp"
#include "ConfigSaving.hpp"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QProcess>
#include <QSettings>
#include <QCoreApplication>
#include <QString>


static QString getSafeTitle(const std::string &title) {
    return QString::fromStdString(title).toLower().replace(" ", "-");
}

static QString getDesktopPath(const std::string &title) {
    return QDir::homePath() + "/.local/share/applications/nativestrapper-" + getSafeTitle(title) + ".desktop";
}

bool URIHandler::InstallURIs(const std::string &title, const std::vector<std::string> &uris) {
#ifdef __linux__
    QString safeTitle = getSafeTitle(title);
    QString desktopPath = getDesktopPath(title);
    QString execPath = QString::fromStdString(ConfigSaving::Current.installDir);
    if (execPath.isEmpty()) { // scary and dangerous
        execPath = QCoreApplication::applicationFilePath();
    }

    // build MimeType string
    QString mimeType;
    for (const auto &uri : uris) {
        mimeType += "x-scheme-handler/" + QString::fromStdString(uri).trimmed() + ";";
    }
    
    QString content =
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=" + QString::fromStdString(title) + "\n"
        "Exec=" + execPath + " --bootstrap-script " + safeTitle + " %u\n"
        "MimeType=" + mimeType + "\n"
        "Terminal=false\n"
        "NoDisplay=true\n";

    QFile file(desktopPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Logger::Log("Failed to write desktop file: " + desktopPath.toStdString(), Logger::LogSeverity::SERROR, "URIHandler");
        return false;
    }
    QTextStream out(&file);
    out << content;
    file.close();

    // register each URI
    bool hasXdgMime = !QStandardPaths::findExecutable("xdg-mime").isEmpty();
    if (hasXdgMime) {
        for (const auto &uri : uris) {
            QString scheme = "x-scheme-handler/" + QString::fromStdString(uri).trimmed();
            QProcess::execute("xdg-mime", {"default", "nativestrapper-" + safeTitle + ".desktop", scheme});
        }
    } else {
        // fallback to mimeapps.list
        QString mimeappsPath = QDir::homePath() + "/.config/mimeapps.list";
        QSettings mimeapps(mimeappsPath, QSettings::IniFormat);
        mimeapps.beginGroup("Default Applications");
        for (const auto &uri : uris) {
            mimeapps.setValue("x-scheme-handler/" + QString::fromStdString(uri).trimmed(), "nativestrapper-" + safeTitle + ".desktop");
        }
        mimeapps.endGroup();
    }

    // checks for the thing first
    bool hasUpdateDb = !QStandardPaths::findExecutable("update-desktop-database").isEmpty();
    if (hasUpdateDb) {
        QProcess::execute("update-desktop-database", {QDir::homePath() + "/.local/share/applications/"});
    }
    Logger::Log("Installed URI handlers for: " + title, Logger::LogSeverity::SSUCCESS, "URIHandler");
    return true;
#endif


    return false;
}

bool URIHandler::UninstallURIs(const std::string &title, const std::vector<std::string> &uris) {
#ifdef __linux__
    QString safeTitle = getSafeTitle(title);
    QString desktopPath = getDesktopPath(title);

    // unregister from mimeapps.list
    QString mimeappsPath = QDir::homePath() + "/.config/mimeapps.list";
    QSettings mimeapps(mimeappsPath, QSettings::IniFormat);
    mimeapps.beginGroup("Default Applications");
    for (const auto &uri : uris) {
        mimeapps.remove("x-scheme-handler/" + QString::fromStdString(uri).trimmed());
    }
    mimeapps.endGroup();

    QFile file(desktopPath);
    if (file.exists()) file.remove();

    bool hasUpdateDb = !QStandardPaths::findExecutable("update-desktop-database").isEmpty();
    if (hasUpdateDb) {
        QProcess::execute("update-desktop-database", {QDir::homePath() + "/.local/share/applications/"});
    }
    Logger::Log("Uninstalled URI handlers for: " + title, Logger::LogSeverity::SSUCCESS, "URIHandler");
    return true;
#endif
    return false;
}