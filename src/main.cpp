#include <QApplication>
#include "UserInterface/OnboardingWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");
    OnboardingWindow w;
    w.show();
    return app.exec();
}