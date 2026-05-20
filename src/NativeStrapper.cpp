#include <string.h>
#include <QApplication>
#include <QPalette>
#include <QMessageBox>
#include "UserInterface/OnboardingWindow.hpp"
#include "VesselManager.hpp"
#include "ConfigSaving.hpp"

struct ArgConfig {
    char* URI = NULL;
    char* VesselTitle = NULL;
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

    ConfigSaving::LoadVessels();

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
        } else if (strcmp(argv[argi], "--vessel") == 0) {
            if (argi + 1 < argc) {
                argConfig.VesselTitle = argv[++argi];
            }
        } else {
            argConfig.URI = argv[argi]; /* anything else is just an URI */
        }
    }
    /*--------------------------------------------------------------*/

    if (argConfig.URI && argConfig.VesselTitle) {
        QString safeArg = QString::fromStdString(std::string(argConfig.VesselTitle)).toLower().replace(" ", "-");

        for (auto &vessel : VesselManager::LoadedVessels) {
            QString safeTitle = QString::fromStdString(vessel.VesselTitle).toLower().replace(" ", "-");

            if (safeTitle == safeArg) {
                std::string cmd = vessel.RobloxRunCommand;
                size_t pos = cmd.find("%u");
                if (pos != std::string::npos) {
                    cmd.replace(pos, 2, argConfig.URI);
                }
                return system(cmd.c_str());
            }
        }
        
        QMessageBox::critical(nullptr, "NativeStrapper", QString("Vessel \"%1\" not found. Please open NativeStrapper and re-import it.").arg(argConfig.VesselTitle));
        return 1;
    }

    // got a URI but no vessel, open GUI
    OnboardingWindow w;
    w.show();
    return app.exec();
}