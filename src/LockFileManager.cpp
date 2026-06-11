#include "LockFileManager.hpp"
#include <QStandardPaths>
#include <QDir>
#ifdef Q_OS_WIN
#  include <windows.h>
#else
#  include <fcntl.h>
#  include <unistd.h>
#  include <sys/file.h>
#endif

namespace LockFileManager {

void LockInstance(const QString &appName) {
#ifdef Q_OS_WIN
    CreateMutexW(nullptr, TRUE, (L"Local\\" + appName.toStdWString()).c_str());
#else
    QString dir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (dir.isEmpty()) dir = QDir::tempPath();
    QString path = dir + "/" + appName + ".lock";

    int fd = open(path.toUtf8().constData(), O_CREAT | O_RDWR, 0600);
    if (fd < 0) return;
    flock(fd, LOCK_EX | LOCK_NB);
#endif
}

bool IsAlreadyRunning(const QString &appName) {
#ifdef Q_OS_WIN
    HANDLE h = CreateMutexW(nullptr, TRUE, (L"Local\\" + appName.toStdWString()).c_str());
    bool running = (GetLastError() == ERROR_ALREADY_EXISTS);
    if (h) CloseHandle(h);
    return running;
#else
    QString dir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (dir.isEmpty()) dir = QDir::tempPath();
    QString path = dir + "/" + appName + ".lock";

    int fd = open(path.toUtf8().constData(), O_CREAT | O_RDWR, 0600);
    if (fd < 0) return false;
    bool running = (flock(fd, LOCK_EX | LOCK_NB) != 0);
    if (!running) flock(fd, LOCK_UN);
    close(fd);
    return running;
#endif
}

}