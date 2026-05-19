#include <string.h>
#include <QApplication>
#include "UserInterface/OnboardingWindow.hpp"

struct ArgConfig {
    char* URI = NULL;
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
    dark.setColor(QPalette::ToolTipText,QColor(221, 221, 221));
    dark.setColor(QPalette::Link, QColor(100, 160, 220));
    dark.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(85, 85, 85));
    dark.setColor(QPalette::Disabled, QPalette::Text, QColor(85, 85, 85));
    app.setPalette(dark);
    app.setStyle("Fusion");
    
    ArgConfig argConfig;
    // second arg should be URI.
    if (argc < 2) {
        OnboardingWindow w;
        w.show();
        return app.exec();
    }

    for (int argi = 0; argi < argc; argi++) {
        if (strcmp(argv[argi], "--test") == 0) {

        } else {
            argConfig.URI = argv[argi];
        }
    }
    return app.exec();
}