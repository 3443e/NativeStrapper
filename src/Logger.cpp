//#include <chrono>
#include <iomanip> // for std::put_time()
#include <iostream>
#include "Logger.hpp"

std::string Logger::GetTime() {
    std::time_t t = std::time(nullptr);

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t), "%H:%M:%S");
    return oss.str();
}

const char* Logger::SeverityToString(Logger::LogSeverity severity) {
    switch (severity) {
        case Logger::LogSeverity::SLOG: 
            return "LOG";
        case Logger::LogSeverity::SINFO:
            return "INFO";
        case Logger::LogSeverity::SWARN:
            return "WARN";
        case Logger::LogSeverity::SERROR:
            return "ERROR";
        case Logger::LogSeverity::SSUCCESS:
            return "INFO";
        case Logger::LogSeverity::SFATAL:
            return "FATAL";
        default:
            return "UNKNOWN";
    }
}

const char* Logger::SeverityColor(Logger::LogSeverity severity) {
    switch (severity) {
        case Logger::LogSeverity::SLOG:
            return "\033[37m"; // gray/white
        case Logger::LogSeverity::SINFO:
            return "\033[36m"; // cyan
        case Logger::LogSeverity::SWARN:
            return "\033[33m"; // yellow
        case Logger::LogSeverity::SERROR:
            return "\033[31m"; // red
        case Logger::LogSeverity::SSUCCESS:
            return "\033[32m"; // green
        case Logger::LogSeverity::SFATAL:
            return "\033[35m"; // magenta
        default:
            return "\033[0m";
    }
}

std::function<void(std::string, Logger::LogSeverity, std::string)> Logger::OnLog = nullptr;

void Logger::Log(std::string message, Logger::LogSeverity severity, std::string from) {
    if (OnLog) OnLog(message, severity, from);
    std::ostringstream out;

    out << "[" << GetTime() << "] ";
    out << SeverityColor(severity);
    out << "[" << SeverityToString(severity) << "]";
    out << "\033[0m";

    if (!from.empty()) {
        out << " [" << from << "]";
    }

    out << " " << message << "\n";

    // send errors to cerr, others to cout
    if (severity == SERROR || severity == SFATAL) {
        std::cerr << out.str();
    } else {
        std::cout << out.str();
    }  
}
