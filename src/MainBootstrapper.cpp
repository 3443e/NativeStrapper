#include "MainBootstrapper.hpp"
#include "ShellUtils.hpp"
#include "Logger.hpp"
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>
#include <fstream>

// launches roblox by running the RobloxRunCommand scatter field.
static int LaunchRoblox(ScatterManager::Scatter* scatter, char* URI) {
    std::string cmd = scatter->RobloxRunCommand;
    size_t pos = cmd.find("%u");
    if (pos == std::string::npos) return 1;
    cmd.replace(pos, 2, URI);
    return system(cmd.c_str());
}

// again got this from somewhere idk
static std::string FetchURL(const std::string &url) {
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QString::fromStdString(url)));
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0");

    QNetworkReply *reply = manager.get(request);

    // block until done
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return "";
    }

    std::string result = reply->readAll().toStdString();
    reply->deleteLater();
    return result;
}

// MainBootstrapper::TokenMap
static std::string ExpandTokens(const std::string &str, const MainBootstrapper::TokenMap &tokens) {
    std::string result = str;
    for (const auto &[token, value] : tokens) {
        std::string key = "%" + token;
        size_t pos;
        while ((pos = result.find(key)) != std::string::npos) {
            result.replace(pos, key.size(), value);
        }
    }
    return result;
}

// entry->token and entry->file handling.
static bool HandleBootstrapVersionEntryOutput(std::string result, ScatterManager::BootstrapVersionEntry& entry, MainBootstrapper::TokenMap &tokens) {
    if (!entry.token.empty()) {
        Logger::Log("Result for token " + entry.token + ": " + result, Logger::LogSeverity::SLOG, "ManageLatestVersionField");
        tokens[entry.token] = result;
    }
    
    if (!entry.file.empty()) {
        Logger::Log("Result for file " + entry.file + ": " + result, Logger::LogSeverity::SLOG, "ManageLatestVersionField");
        std::string filePath = ExpandTokens(entry.file, tokens);
        std::ofstream f(filePath);
        if (!f.is_open()) {
            Logger::Log("Failed to write to file: " + filePath, Logger::LogSeverity::SERROR, "ManageLatestVersionField");
            return false;
        }
        f << result;
        f.close();
        Logger::Log("Saved response to: " + filePath, Logger::LogSeverity::SINFO, "ManageLatestVersionField");
    }
    return true;
}

static bool RunCurrentVersionCommands(ScatterManager::Scatter* scatter, MainBootstrapper::TokenMap &tokens) {
    for (auto &entry : scatter->Bootstrap.CurrentVersion) {
        std::string cmd = ExpandTokens(entry.system, tokens);
        std::string result = ShellUtils::RunCommand(cmd);
        if (result.empty()) {
            Logger::Log("Command didn't print anything, bad command?", Logger::LogSeverity::SWARN, "RunCurrentVersionCommands");
        }

        if (!HandleBootstrapVersionEntryOutput(result, entry, tokens)) {
            Logger::Log("Bad output format on CurrentVersion, please use 'token' or 'file'.", Logger::LogSeverity::SERROR, "HandleBootstrapVersionEntryOutput");
            return false;
        }
    }
    return true;
}

static bool ManageLatestVersionField(ScatterManager::Scatter* scatter, MainBootstrapper::TokenMap &tokens) {
    for (auto &entry : scatter->Bootstrap.LatestVersion) {
        if (entry.token.empty() && entry.file.empty()) { /* this shouldn't happen because we have checks on ScatterManager::ParseScatter() */
            Logger::Log("LatestVersion entry has neither token nor file, skipping", Logger::LogSeverity::SERROR, "ManageLatestVersionField");
            return false;
        }
        std::string result = "";

        if (!entry.url.empty()) {
            std::string url = ExpandTokens(entry.url, tokens);
            Logger::Log("Fetching URL: " + url, Logger::LogSeverity::SINFO, "ManageLatestVersionField");
            result = FetchURL(url);
            if (result.empty()) {
                Logger::Log("URL fetch returned empty: " + url, Logger::LogSeverity::SERROR, "ManageLatestVersionField");
                return false;
            }
            Logger::Log("URL fetch got " + std::to_string(result.size()) + " bytes", Logger::LogSeverity::SINFO, "ManageLatestVersionField");
            if (!HandleBootstrapVersionEntryOutput(result, entry, tokens)) {
                Logger::Log("Bad output format on LatestVersion, please use 'token' or 'file'.", Logger::LogSeverity::SERROR, "HandleBootstrapVersionEntryOutput");
                return false;
            }
        }

        if (!entry.system.empty()) {
            std::string cmd = ExpandTokens(entry.system, tokens);
            std::string cmdresult = ShellUtils::RunCommand(cmd);
            if (cmdresult.empty()) {
                Logger::Log("Command returned empty: ", Logger::LogSeverity::SERROR, "ManageLatestVersionField");
                return false;
            }
            
            result = cmdresult;
        }

        // second run for command output
        if (!HandleBootstrapVersionEntryOutput(result, entry, tokens)) {
            Logger::Log("Bad output format on LatestVersion, please use 'token' or 'file'.", Logger::LogSeverity::SERROR, "HandleBootstrapVersionEntryOutput");
            return false;
        }
    }
    return true;
}

MainBootstrapper::MainStartResult MainBootstrapper::StartStrappin(ScatterManager::Scatter* scatter, char* URI) {
    // properly configured bootstrap stuff
    if (scatter->HasBootstrap) {
        TokenMap tokens;

        bool CurrentVerResult = RunCurrentVersionCommands(scatter, tokens);
        if (!CurrentVerResult) {
            Logger::Log("Failed to get current version, Launching anyway", Logger::LogSeverity::SERROR, "MainBootstrapper");
            // TODO: show some kind of warning
        }

        bool ManageLatestVerResult = ManageLatestVersionField(scatter, tokens);
        if (!ManageLatestVerResult) {
            Logger::Log("Failed to get latest version info, Launching anyway", Logger::LogSeverity::SERROR, "MainBootstrapper");
            // TODO: show some kind of warning
        }
    }

    // Launch roblox
    int LaunchResult = LaunchRoblox(scatter, URI);
    if (LaunchResult != 0) {
        return MainBootstrapper::MainStartResult::BOOTSTRAPLAUNCHERROR;
    }
    return MainBootstrapper::MainStartResult::BOOTSTRAPSUCCESS;
}