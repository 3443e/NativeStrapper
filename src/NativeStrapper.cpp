#include <iostream>
#include <thread>
#include <string>
#include <QApplication>
#include <QPalette>
#include <QMessageBox>
#include "ConfigSaving.hpp"
#include "NativeStrapper/Installer.hpp"
#include "UserInterface/InstallerWindow.hpp"
#include "UserInterface/BootstrapWindow.hpp"
#include "UserInterface/OnboardingWindow.hpp"
#include "BootstrapScripts/ScriptManager.hpp"
#include "BootstrapScripts/LuaScript.hpp"
#include "BootstrapScripts/ScriptManager.hpp"
#include "Logger.hpp"
#include "NativeStrapper.hpp"

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

struct ArgConfig {
    char* URI = NULL;
    char* BootstrapScript = NULL;
};

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
    Logger::Log("Testing lualib...", Logger::LogSeverity::SLOG, "NativeStrapperMain");

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_dostring(L, "print('[xx:xx:xx] [LOG] [LuaNativeStrapperMain] Hello from dostring')");
    lua_close(L);

    Logger::Log("Creating qapplication", Logger::LogSeverity::SLOG, "NativeStrapperMain");
    QApplication app(argc, argv);

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

    Logger::Log("Loading saved config config", Logger::LogSeverity::SLOG, "NativeStrapperMain");

    // loads config json
    ConfigSaving::Load();

    if (!Installer::IsInstalled() && !Installer::IsRunningFromInstallLocation()) {
        Logger::Log("NativeStrapper not installed, launching starting setup", Logger::LogSeverity::SINFO, "NativeStrapperMain");
        InstallerWindow w;
        w.show();
        return app.exec();
    }
    
    /*
    if (!Installer::IsRunningFromInstallLocation()) {
        Logger::Log("NativeStrapper is already installed. Please launch it from where you've installed it.", Logger::LogSeverity::SFATAL, "NativeStrapperMain");
        QMessageBox::critical(nullptr, "Error", "NativeStrapper is already installed. Please launch it from where you've installed it.");
        return 1;
    }
    */
    
    // moooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
    Logger::Log("Loading installed bootstrap scripts", Logger::LogSeverity::SLOG, "NativeStrapperMain");
    ScriptManager::LoadScripts(); /* scans the installed-scripts folder and loads everything to ScriptManager::LoadedScripts*/
    /* it also reinstalls all URIs */

    // Argument parsing section
    /*--------------------------------------------------------------*/
    ArgConfig argConfig;

    // no args, just show the GUI
    if (argc < 2) {
        OnboardingWindow w;
        w.show();
        return app.exec();
    }

    // parse args
    for (int argi = 1; argi < argc; argi++) {
        if (strcmp(argv[argi], "--test") == 0) {
            // placeholder
        } else if (strcmp(argv[argi], "--bootstrap-script") == 0) {
            if (argi + 1 < argc) {
                argConfig.BootstrapScript = argv[++argi];
            }
        } else {
            argConfig.URI = argv[argi]; /* anything else is just an URI */
        }
    }
    /*--------------------------------------------------------------*/

    
    if (argConfig.URI && argConfig.BootstrapScript) {
        Logger::Log("Launched with bootstrap script and URI.", Logger::LogSeverity::SLOG, "NativeStrapperMain");
        QString safeArg = QString::fromStdString(std::string(argConfig.BootstrapScript)).toLower().replace(" ", "-");

        for (auto &script : ScriptManager::LoadedScripts) {
            QString safeTitle = QString::fromStdString(script.title).toLower().replace(" ", "-");

            if (safeTitle == safeArg) {
                Logger::Log("Found bootstrap script, creating bootstrapper window", Logger::LogSeverity::SLOG, "NativeStrapperMain");
                BootstrapWindow *w = new BootstrapWindow();
                w->show();

                Logger::OnLog = [w](std::string message, Logger::LogSeverity severity, std::string from) {
                    QMetaObject::invokeMethod(w, [w, message]() {
                        w->setLog(QString::fromStdString(message));
                    }, Qt::QueuedConnection);
                };
                
                Logger::Log("Executing bootstrap script...", Logger::LogSeverity::SINFO, "NativeStrapperMain");
                
                // run the bootstrap script in a thread
                std::thread scriptThread([&script, &argConfig, w]() {
                    std::string scriptPath = ScriptManager::GetScriptPath(script.title);
                    LuaScript::RunScript(scriptPath, std::string(argConfig.URI), w);
                });

                // ye
                scriptThread.detach();

                return app.exec();
            }
        }
        Logger::Log("Couldn't find bootstrap script.", Logger::LogSeverity::SFATAL, "NativeStrapperMain");
        QMessageBox::critical(nullptr, "NativeStrapper", QString("Bootstrap script \"%1\" not found. Please open NativeStrapper and re-import it.").arg(argConfig.BootstrapScript));
        return 1;
    }
    
    // got a URI but no scatter, open GUI
    OnboardingWindow w;
    w.show();
    return app.exec();
}