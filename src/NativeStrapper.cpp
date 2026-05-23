#include <thread>
#include <string.h>
#include <QApplication>
#include <QPalette>
#include <QMessageBox>
#include "UserInterface/BootstrapWindow.hpp"
#include "UserInterface/OnboardingWindow.hpp"
#include "BootstrapScripts/ScriptManager.hpp"
#include "BootstrapScripts/LuaScript.hpp"
#include "BootstrapScripts/ScriptManager.hpp"
#include "Logger.hpp"

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
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_dostring(L, "print('lua works!!')");
    lua_close(L);
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

    // moooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
    ScriptManager::LoadScripts(); /* scans the installed-scripts folder and loads everything to ScriptManager::LoadedScripts*/

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
        QString safeArg = QString::fromStdString(std::string(argConfig.BootstrapScript)).toLower().replace(" ", "-");

        for (auto &script : ScriptManager::LoadedScripts) {
            QString safeTitle = QString::fromStdString(script.title).toLower().replace(" ", "-");

            if (safeTitle == safeArg) {
                BootstrapWindow *w = new BootstrapWindow();
                w->show();

                Logger::OnLog = [w](std::string message, Logger::LogSeverity severity, std::string from) {
                    QMetaObject::invokeMethod(w, [w, message]() {
                        w->setLog(QString::fromStdString(message));
                    }, Qt::QueuedConnection);
                };
                
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

        QMessageBox::critical(nullptr, "NativeStrapper", QString("Bootstrap script \"%1\" not found. Please open NativeStrapper and re-import it.").arg(argConfig.BootstrapScript));
        return 1;
    }
    
    // got a URI but no scatter, open GUI
    OnboardingWindow w;
    w.show();
    return app.exec();
}