/*
-------------------NativeStrapper--------------------
A cross-platform and very modular roblox bootstrapper.

something something
*/

#include <iostream>
#include <thread>
#include <string>
#include <QApplication>
#include <QPalette>
#include <QMessageBox>
#include <QProcess>
#include "ConfigSaving.hpp"
#include "BootstrapScripts/ScriptManager.hpp"
#include "BootstrapScripts/ScriptManager.hpp"
#include "DiscordRPC/ActivityWatcher.hpp"

#include "Logger.hpp"
#include "NativeStrapper.hpp"
#include "Bootstrapper.hpp"

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

int main(int argc, char *argv[]) {
    std::cout << "\033[36mNativeStrapper\033[0m v" + std::string(NativeStrapper::NativeStrapperVersion) << std::endl;
    std::cout << "Repo: " + std::string(NativeStrapper::NativeStrapperRepo) << std::endl;
    std::cout << "-------------------------------------------" << std::endl;
    std::cout << "\033[36m" << R"(
            ;;;;
           ;;;;;;;;;;;;
           ;;;;;;;;;;;;;;;;  ;
          ;;;;;;;;;;;;;;;;  ;;;;;;;;;
          ;;;;;;;;;;;;;;;   ;;;;;;;;;;;;;;
         ;;;;;;;;;;;;;;;;  ;;;;;;;;;;;;;;;;
         ;;;;;;;;;;;;;;;   ;;;;;;;;;;;;;;;
        ;;;;;;;;;;;;;;;;  ;;;;;;;;;;;;;;;;
             ;;;;;;;       ;;;;;;;;;;;;;;
       ;;;;;;                ;;;;;;;;;;;;
       ;;;;;;;;;;;;         ;;;;;;;;;;;;
      ;;;;;;;;;;;;;               ;;;;;;
      ;;;;;;;;;;;;;;;      ;;;;;;;
     ;;;;;;;;;;;;;;;;  ;;;;;;;;;;;;;;;;
     ;;;;;;;;;;;;;;;   ;;;;;;;;;;;;;;;
    ;;;;;;;;;;;;;;;;  ;;;;;;;;;;;;;;;;
     ;;;;;;;;;;;;;;   ;;;;;;;;;;;;;;;
          ;;;;;;;;;  ;;;;;;;;;;;;;;;;
                 ;   ;;;;;;;;;;;;;;;
                        ;;;;;;;;;;;;
                               ;;;;
    )" << "\033[0m" << std::endl;
    std::cout << "-------------------------------------------" << std::endl;

    NativeStrapper::ArgConfig argConfig;

    for (int argi = 1; argi < argc; argi++) {
        if (strcmp(argv[argi], "--activity-watch") == 0) {
            argConfig.ActivityWatch = true;
        } else if (strcmp(argv[argi], "--bootstrap-script") == 0) {
            if (argi + 1 < argc) {
                argConfig.BootstrapScript = argv[++argi];
            }
        } else {
            argConfig.URI = argv[argi]; /* anything else is just an URI */
        }
    }

    // ~/.config/nativestrapper/
    QCoreApplication::setApplicationName("nativestrapper");

    if (argConfig.ActivityWatch) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        Logger::Log("Loading config", Logger::LogSeverity::SLOG, "NativeStrapperMain");
        ConfigSaving::Load();
        
        Logger::Log("Loading installed bootstrap scripts", Logger::LogSeverity::SLOG, "NativeStrapperMain");
        ScriptManager::LoadScripts(false);

        ScriptManager::BootstrapScript* bootstrapScript = ScriptManager::FindFirstScriptByName(argConfig.BootstrapScript);
        if (bootstrapScript == NULL) {
            Logger::Log("Couldn't find script, activity watcher will exit.", Logger::LogSeverity::SLOG, "NativeStrapperMain");
            return 1;
        }
        
        Logger::Log("Starting watcher thread for " + std::string(argConfig.BootstrapScript), Logger::LogSeverity::SLOG, "NativeStrapperMain");
        ActivityWatcher::StartWatcherThread(bootstrapScript);
        
        std::this_thread::sleep_for(std::chrono::hours(67));
    }

    Logger::Log("Creating QApplication", Logger::LogSeverity::SLOG, "NativeStrapperMain");
    QApplication app(argc, argv);

    Logger::Log("Setting theme", Logger::LogSeverity::SLOG, "NativeStrapperMain");
    QPalette dark;
    dark.setColor(QPalette::Window, QColor(38, 38, 38));
    dark.setColor(QPalette::WindowText, QColor(221, 221, 221));
    dark.setColor(QPalette::Base, QColor(30, 30, 30));
    dark.setColor(QPalette::AlternateBase, QColor(45, 45, 45));
    dark.setColor(QPalette::Text, QColor(221, 221, 221));
    dark.setColor(QPalette::Button, QColor(45, 45, 45));
    dark.setColor(QPalette::ButtonText, QColor(221, 221, 221));
    dark.setColor(QPalette::Highlight, QColor(68, 68, 68));
    dark.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    dark.setColor(QPalette::ToolTipBase, QColor(45, 45, 45));
    dark.setColor(QPalette::ToolTipText, QColor(221, 221, 221));
    dark.setColor(QPalette::Link, QColor(100, 160, 220));
    dark.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(85, 85, 85));
    dark.setColor(QPalette::Disabled, QPalette::Text, QColor(85, 85, 85));
    app.setPalette(dark);
    app.setStyle("Fusion");

    Logger::Log("Loading saved config.json", Logger::LogSeverity::SLOG, "NativeStrapperMain");
    ConfigSaving::Load();

    Logger::Log("Loading installed bootstrap scripts", Logger::LogSeverity::SLOG, "NativeStrapperMain");
    ScriptManager::LoadScripts(false); /* scans the installed-scripts folder and loads everything to ScriptManager::LoadedScripts */

#if defined(_WIN32)
    Logger::Log("Registering URI handlers on Windows", Logger::LogSeverity::SLOG, "NativeStrapperMain");
    bool ok1 = platform::SetAsDefaultHandler("roblox");
    bool ok2 = platform::SetAsDefaultHandler("roblox-player");
    if (ok1 || ok2) {
        Logger::Log("URI registration attempted (some results may require user confirmation)", Logger::LogSeverity::SLOG, "NativeStrapperMain");
    } else {
        Logger::Log("Failed to register URI handlers on Windows", Logger::LogSeverity::SWARN, "NativeStrapperMain");
    }
#endif

    Bootstrapper::BootstrapResult BResult = Bootstrapper::MainBootstrap(&argConfig); // This will create a BootstrapWindow.
    if (BResult != Bootstrapper::BootstrapResult::BOOTSTRAP_SUCCESS) {
        QMessageBox::critical(nullptr, "NativeStrapper", QString::fromUtf8(Bootstrapper::Exception.c_str()));
        return BResult;
    }

    return app.exec();
}
