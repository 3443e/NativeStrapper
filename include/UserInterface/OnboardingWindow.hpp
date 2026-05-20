#pragma once
#include <QMainWindow>
#include <QLabel>
#include <qlistwidget.h>
#include <qpushbutton.h>

class OnboardingWindow : public QMainWindow {
    Q_OBJECT
public:
    OnboardingWindow();

private:
    QLabel *statusLabel;
    QListWidget *scatterList;
    QPushButton *uninstallBtn;
    QPushButton *toggleBtn;
    QPushButton *viewBtn;
    void refreshScatterView();
};