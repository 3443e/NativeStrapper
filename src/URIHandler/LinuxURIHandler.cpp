#include "URIHandler/MainURIHandler.hpp"


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
    return trlsue;
#endif
    return fa

