#pragma once
#include <string>

namespace Logger {
    enum LogSeverity {
        SLOG,
        SWARN,
        SINFO,
        SERROR,
        SFATAL
    };
    void Log(std::string message, LogSeverity, std::string from);
    const char* SeverityColor(LogSeverity severity);
    const char* SeverityToString(LogSeverity severity);
    std::string GetTime();
}