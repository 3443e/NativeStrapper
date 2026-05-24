#pragma once
#include <QString>

namespace Installer {
    bool IsInstalled();
    bool IsRunningFromInstallLocation();
    QString GetDefaultInstallLocation();
    bool Install(std::string installDir);
    bool Uninstall();
}