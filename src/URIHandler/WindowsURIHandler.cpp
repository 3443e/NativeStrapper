#ifdef Q_OS_WIN
#include "URIHandler/MainURIHandler.hpp"
#include "Logger.hpp"
#include "ConfigSaving.hpp"
#include <QCoreApplication>
#include <QString>
#include <windows.h>

static void setRegString(HKEY key, const QString &name, const QString &value) {
    std::wstring wname = name.toStdWString();
    std::wstring wvalue = value.toStdWString();
    RegSetValueExW(key, wname.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(wvalue.c_str()), (wvalue.size() + 1) * sizeof(wchar_t));
}

bool URIHandler::InstallURIs(const std::string &title, const std::vector<std::string> &uris) {
    QString execPath = QString::fromStdString(ConfigSaving::Current.installDir);
    if (execPath.isEmpty()) {
        execPath = QCoreApplication::applicationFilePath();
    }
    execPath = QDir::toNativeSeparators(execPath);

    for (const auto &uri : uris) {
        QString scheme = QString::fromStdString(uri).trimmed();
        QString keyPath = "Software\\Classes\\" + scheme;

        HKEY uriKey, iconKey, commandKey;

        RegCreateKeyExW(HKEY_CURRENT_USER, keyPath.toStdWString().c_str(), 0, nullptr, 0, KEY_SET_VALUE | KEY_CREATE_SUB_KEY, nullptr, &uriKey, nullptr);

        setRegString(uriKey, "", "URL: " + scheme + " Protocol");
        setRegString(uriKey, "URL Protocol", "");

        RegCreateKeyExW(HKEY_CURRENT_USER, (keyPath + "\\DefaultIcon").toStdWString().c_str(), 0, nullptr, 0, KEY_SET_VALUE, nullptr, &iconKey, nullptr);
        setRegString(iconKey, "", execPath);
        RegCloseKey(iconKey);

        RegCreateKeyExW(HKEY_CURRENT_USER, (keyPath + "\\shell\\open\\command").toStdWString().c_str(), 0, nullptr, 0, KEY_SET_VALUE, nullptr, &commandKey, nullptr);
        setRegString(commandKey, "", "\"" + execPath + "\" \"%1\"");
        RegCloseKey(commandKey);

        RegCloseKey(uriKey);
    }

    Logger::Log("Installed URI handlers for: " + title, Logger::LogSeverity::SSUCCESS, "URIHandler");
    return true;
}

bool URIHandler::UninstallURIs(const std::string &title, const std::vector<std::string> &uris) {
    for (const auto &uri : uris) {
        QString scheme = QString::fromStdString(uri).trimmed();
        QString keyPath = "Software\\Classes\\" + scheme;

        RegDeleteTreeW(HKEY_CURRENT_USER, keyPath.toStdWString().c_str());
    }

    Logger::Log("Uninstalled URI handlers for: " + title, Logger::LogSeverity::SSUCCESS, "URIHandler");
    return true;
}
#endif