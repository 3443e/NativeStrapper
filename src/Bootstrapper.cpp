#include <thread>
#include "Bootstrapper.hpp"
#include "BootstrapScripts/LuaScript.hpp"
#include "BootstrapScripts/ScriptManager.hpp"
#include "Logger.hpp"
#include "UserInterface/BootstrapWindow.hpp"
#include "UserInterface/OnboardingWindow.hpp"
#include <QCoreApplication>
#include <QProcess>
#include <QMessageBox>

std::string Bootstrapper::Exception = "";

Bootstrapper::BootstrapResult Bootstrapper::MainBootstrap(NativeStrapper::ArgConfig* argConfig) {
    if (argConfig->URI && argConfig->BootstrapScript) {
        Logger::Log("Launched with bootstrap script and URI.", Logger::LogSeverity::SLOG, "MainBootstrap");
        QString safeTitle = QString::fromStdString(std::string(argConfig->BootstrapScript)).toLower().replace(" ", "-");

        ScriptManager::BootstrapScript* script = ScriptManager::FindFirstScriptByName(argConfig->BootstrapScript);
        if (script == NULL) {
            Logger::Log("Couldn't find bootstrap script.", Logger::LogSeverity::SFATAL, "MainBootstrap");
            Bootstrapper::Exception = QString("Bootstrap script \"%1\" not found. Please open NativeStrapper and re-import it.").arg(argConfig->BootstrapScript).toStdString();
            return Bootstrapper::BootstrapResult::BOOTSTRAP_SCRIPT_NOT_FOUND;
        }

        Logger::Log("Found bootstrap script, creating bootstrapper window", Logger::LogSeverity::SLOG, "MainBootstrap");
        BootstrapWindow *w = new BootstrapWindow();
        w->show();

        Logger::OnLog = [w](std::string message, Logger::LogSeverity severity, std::string from) {
            QMetaObject::invokeMethod(w, [w, message]() {
                w->setLog(QString::fromStdString(message));
            }, Qt::QueuedConnection);
        };

        Logger::Log("Executing bootstrap script...", Logger::LogSeverity::SINFO, "MainBootstrap");
        
        std::string scriptPath = ScriptManager::GetScriptPath(script->title);
        std::string uri = std::string(argConfig->URI);
        std::string runCmd = script->run;

        size_t pos = runCmd.find("%u");
        if (pos != std::string::npos) {
            runCmd.replace(pos, 2, uri);
        }

        std::thread scriptThread([scriptPath, uri, w, runCmd, safeTitle]() {
            bool RunResult = LuaScript::RunScript(scriptPath, uri, w);
            if (!RunResult) {
                Logger::Log("Script experienced an error while running.", Logger::LogSeverity::SFATAL, "MainBootstrap");
                Bootstrapper::Exception = "Script experienced an error while running.";
                return Bootstrapper::BootstrapResult::BOOTSTRAP_SCRIPT_ERR;
            }

            // calling these functions directly causes a segfault, smh... SMH. SHAKING MY HEAD, SHAKING MY FUCKING HEAD
            QMetaObject::invokeMethod(w, [w]() {
                w->setStatus("Starting Roblox...");
                w->setIndeterminate(true);
            }, Qt::QueuedConnection);
            
            QStringList parts = QProcess::splitCommand(
                QString::fromStdString(runCmd)
            );
            QString robloxexepath = parts.takeFirst();
            QString exePath = QCoreApplication::applicationFilePath();

            bool robloxStarted = QProcess::startDetached(robloxexepath, parts);

            if (robloxStarted) {
                QProcess::startDetached(exePath, {"--bootstrap-script", safeTitle, "--activity-watch"});
                QMetaObject::invokeMethod(qApp,[]() {
                    QCoreApplication::quit();
                }, Qt::QueuedConnection);
            }
        });

        scriptThread.detach();

        return Bootstrapper::BootstrapResult::BOOTSTRAP_SUCCESS;
    } else {
        auto* w = new OnboardingWindow(); /* putting onboarding window on the stack causes it to not show */
        w->show();
    }

    return Bootstrapper::BootstrapResult::BOOTSTRAP_SUCCESS;
}