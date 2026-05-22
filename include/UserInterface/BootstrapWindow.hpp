#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>

class BootstrapWindow : public QMainWindow {
    Q_OBJECT
public:
    void setLog(const QString &text);
    void setProgress(int value);
    void setStatus(const QString &text);
    BootstrapWindow();

private:
    QLabel *statusLabel;
    QLabel *logLabel;
    QLabel *speedLabel;
    QLabel *etaLabel;
    QProgressBar *progressBar;

protected:
    void closeEvent(QCloseEvent *event) override;
};