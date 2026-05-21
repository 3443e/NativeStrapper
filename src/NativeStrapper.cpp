#include <string.h>
#include <QApplication>
#include <QPalette>
#include <QMessageBox>
#include "UserInterface/OnboardingWindow.hpp"
#include "ScatterManager.hpp"
#include "ConfigSaving.hpp"
#include "MainBootstrapper.hpp"

struct ArgConfig {
    char* URI = NULL;
    char* ScatterTitle = NULL;
};

int main(int argc, char *argv[]) {
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

    ConfigSaving::LoadScatters();

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
        } else if (strcmp(argv[argi], "--scatter") == 0) {
            if (argi + 1 < argc) {
                argConfig.ScatterTitle = argv[++argi];
            }
        } else {
            argConfig.URI = argv[argi]; /* anything else is just an URI */
        }
    }
    /*--------------------------------------------------------------*/

    if (argConfig.URI && argConfig.ScatterTitle) {
        QString safeArg = QString::fromStdString(std::string(argConfig.ScatterTitle)).toLower().replace(" ", "-");
        for (auto &scatter : ScatterManager::LoadedScatters) {
            QString safeTitle = QString::fromStdString(scatter.ScatterTitle).toLower().replace(" ", "-");
            // find which installed scatter file was summoned (ok)
            if (safeTitle == safeArg) {
                MainBootstrapper::MainStartResult result = MainBootstrapper::StartStrappin(&scatter, argConfig.URI);
                return 0;
            }
        }
        
        QMessageBox::critical(nullptr, "NativeStrapper", QString("Scatter \"%1\" not found. Please open NativeStrapper and re-import it.").arg(argConfig.ScatterTitle));
        return 1;
    }

    // got a URI but no scatter, open GUI
    OnboardingWindow w;
    w.show();
    return app.exec();
}