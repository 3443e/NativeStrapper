#include "NativeStrapper/Installer.hpp"
#include "ConfigSaving.hpp"
#include "Logger.hpp"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>

QString Installer::GetDefaultInstallLocation() {
#ifdef __linux__
    return QDir::homePath() + "/.local/bin/nativestrapper";
#elif _WIN32
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/NativeStrapper/NativeStrapper.exe";
#else
    return QDir::homePath() + "/NativeStrapper";
#endif
}

bool Installer::IsInstalled() {
    std::string installDir = "";
    if (!ConfigSaving::Current.installDir.empty()) installDir = ConfigSaving::Current.installDir;
    return QFile::exists(QString::fromStdString(installDir));
}

bool Installer::IsRunningFromInstallLocation() {
    QString currentPath = QCoreApplication::applicationFilePath();
    std::string installDir = "";
    if (!ConfigSaving::Current.installDir.empty()) installDir = ConfigSaving::Current.installDir;
    return (QFileInfo(currentPath) == QFileInfo(QString::fromStdString(installDir)));
}

bool Installer::Install(std::string installDir) {
    QString src = QCoreApplication::applicationFilePath();
    QString dest = QString::fromStdString(installDir);

    QDir().mkpath(QFileInfo(dest).absolutePath()); /* creates folders and stuff */
    if (QFile::exists(dest)) {
        QFile::remove(dest);
    }

    if (!QFile::copy(src, dest)) {
        Logger::Log("Failed to copy binary to: " + dest.toStdString(), Logger::LogSeverity::SERROR, "Installer");
        return false;
    }

    /* Linux permission bs */
#ifdef __linux__
    QFile::setPermissions(dest, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner | QFile::ReadGroup | QFile::ExeGroup | QFile::ReadOther | QFile::ExeOther);
#endif

    // save install location
    ConfigSaving::Current.installDir = dest.toStdString();

    Logger::Log("Installed to: " + dest.toStdString(), Logger::LogSeverity::SSUCCESS, "Installer");
    return true;
}

bool Installer::Uninstall() {
    std::string installDir = "";
    if (!ConfigSaving::Current.installDir.empty()) installDir = ConfigSaving::Current.installDir;

    if (QFile::exists(QString::fromStdString(installDir))) {
        QFile::remove(QString::fromStdString(installDir));
    }

    ConfigSaving::Current.installDir = "";

    Logger::Log("Uninstalled from: " + installDir, Logger::LogSeverity::SSUCCESS, "Installer");
    return true;
}