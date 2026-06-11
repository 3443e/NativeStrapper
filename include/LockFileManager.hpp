#pragma once
#include <QString>

namespace LockFileManager {
    void LockInstance(const QString &appName);
    bool IsAlreadyRunning(const QString &appName);
}