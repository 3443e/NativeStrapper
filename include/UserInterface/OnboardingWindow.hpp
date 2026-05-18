#pragma once
#include <QMainWindow>
#include <QLabel>

class OnboardingWindow : public QMainWindow {
    Q_OBJECT
public:
    OnboardingWindow();

private:
    QLabel *statusLabel;
};