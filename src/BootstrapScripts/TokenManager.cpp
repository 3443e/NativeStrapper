#include "BootstrapScripts/TokenManager.hpp"

#include <QString>
#include <QDir>

std::string TokenManager::TokenizeString(std::string untokenized) {
    QString result = QString::fromStdString(untokenized);
    const QString homePath = QDir::homePath();

    QString user = qEnvironmentVariable("USER");
    if (user.isEmpty()) { /* mmm */
        user = qEnvironmentVariable("USERNAME");
    }

    result.replace("%home", homePath);
    result.replace("%user", user);

    return result.toStdString();
}