#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class InstallerWindow : public QMainWindow {
    Q_OBJECT
public:
    InstallerWindow();
private:
    QLineEdit *pathEdit;
    QPushButton *installBtn;
    void doInstall();
};