#pragma once
#include <string>
#include <functional>

namespace Logger {
    enum LogSeverity {
        SLOG,
        SINFO,
        SWARN,
        SERROR,
        SSUCCESS,
        SFATAL
    };

    extern std::function<void(std::string, LogSeverity, std::string)> OnLog;

    std::string GetTime();
    const char* SeverityToString(LogSeverity severity);
    const char* SeverityColor(LogSeverity severity);
    void Log(std::string message, LogSeverity severity, std::string from = "");
}