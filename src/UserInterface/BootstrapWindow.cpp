#include "UserInterface/BootstrapWindow.hpp"
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QProgressBar>
#include <QCoreApplication>
#include <QPushButton>
#include <QCloseEvent>

BootstrapWindow::BootstrapWindow() {
    setWindowTitle("NativeStrapper");
    setFixedSize(400, 240);

    auto *root = new QWidget();
    auto *layout = new QVBoxLayout(root);
    layout->setContentsMargins(0, 20, 0, 20);
    layout->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    layout->setSpacing(6);

    layout->addStretch(5);
    QLabel *imageLabel = new QLabel();
    QPixmap pixmap(QCoreApplication::applicationDirPath() + "/assets/NativeStrapper.png");
    imageLabel->setPixmap(pixmap.scaled(160, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(imageLabel, 0, Qt::AlignHCenter);

    layout->addSpacing(8);

    statusLabel = new QLabel("Starting Roblox...");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("font-size: 18px; color: #cccccc;");
    layout->addWidget(statusLabel, 0, Qt::AlignHCenter);

    layout->addSpacing(4);

    progressBar = new QProgressBar();
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(false);
    progressBar->setFixedWidth(360);
    progressBar->setFixedHeight(3);
    progressBar->setStyleSheet(
        "QProgressBar {"
        "  background-color: #333333;"
        "  border-radius: 1px;"
        "  border: none;"
        "}"
        "QProgressBar::chunk {"
        "  background-color: #888888;"
        "  border-radius: 1px;"
        "}"
    );
    layout->addWidget(progressBar, 0, Qt::AlignHCenter);

    layout->addSpacing(4);

    logLabel = new QLabel("HELLO! :3");
    logLabel->setAlignment(Qt::AlignCenter);
    logLabel->setStyleSheet("font-size: 9px; color: rgba(255, 255, 255, 80);");
    logLabel->setWordWrap(true);
    logLabel->setFixedWidth(360);
    layout->addWidget(logLabel, 0, Qt::AlignHCenter);

    layout->addSpacing(6);

    QPushButton *cancelBtn = new QPushButton("Cancel");
    cancelBtn->setFixedWidth(80);
    QObject::connect(cancelBtn, &QPushButton::clicked, [&]() {
        this->close();
        exit(0);
    });
    layout->addWidget(cancelBtn, 0, Qt::AlignHCenter);

    setCentralWidget(root);
}

void BootstrapWindow::closeEvent(QCloseEvent *event) {
    event->accept();
    exit(0);
}

void BootstrapWindow::setLog(const QString &text) {
    logLabel->setText(text);
}

void BootstrapWindow::setStatus(const QString &text) {
    statusLabel->setText(text);
}

void BootstrapWindow::setProgress(int value) {
    progressBar->setValue(value);
}