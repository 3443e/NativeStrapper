#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>

class BootstrapWindow : public QMainWindow {
    Q_OBJECT
public:
    void setLog(const QString &text);
    void setProgress(int value);
    void setStatus(const QString &text);
    BootstrapWindow();
    void setIndeterminate(bool indeterminate);
private:
    QLabel *statusLabel;
    QLabel *logLabel;
    QLabel *speedLabel;
    QLabel *etaLabel;
    QProgressBar *progressBar;

    QTimer *indeterminateTimer;
    int indeterminateValue = 0;
    bool isIndeterminate = true;
protected:
    void closeEvent(QCloseEvent *event) override;
};