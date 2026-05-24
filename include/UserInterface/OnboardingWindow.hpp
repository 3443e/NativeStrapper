#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

class OnboardingWindow : public QMainWindow {
    Q_OBJECT
public:
    OnboardingWindow();
private:
    QLabel *statusLabel;
    QListWidget *scriptList;
    QPushButton *viewBtn;
    QPushButton *uninstallBtn;
    QPushButton *launchBtn;
    QPushButton *settingsBtn;
    void refreshScriptView();
};