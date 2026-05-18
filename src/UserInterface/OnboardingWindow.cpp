#include "UserInterface/OnboardingWindow.hpp"
#include "UserInterface/SettingsWindow.hpp"
#include <QApplication>
#include <QCoreApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>

OnboardingWindow::OnboardingWindow() {
    setWindowTitle("NativeStrapper");
    setFixedSize(400, 240);

    auto *root = new QWidget();
    auto *layout = new QHBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *leftWidget = new QWidget();
    leftWidget->setStyleSheet("background-color: #262626;");
    auto *left = new QVBoxLayout(leftWidget);

    left->addWidget(new QPushButton("Launch Roblox"));

    QPushButton *settingsBtn = new QPushButton("Settings", leftWidget);
    QObject::connect(settingsBtn, &QPushButton::clicked, [&]() {
        SettingsWindow *window = new SettingsWindow();
        window->show();
        this->hide();
    });
    left->addWidget(settingsBtn);

    QPushButton *quitBtn = new QPushButton("Quit", leftWidget);
    QObject::connect(quitBtn, &QPushButton::clicked, [&]() {
        this->close();
        exit(0);
    });
    left->addWidget(quitBtn);

    left->addStretch();
    auto *bottomText = new QLabel("Made with love -3443");
    bottomText->setStyleSheet("font-size: 10px; color: #888888;");
    left->addWidget(bottomText);

    layout->addWidget(leftWidget);

    QWidget *rightWidget = new QWidget();
    rightWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 20, 0, 0);
    rightLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QLabel *imageLabel = new QLabel();
    QPixmap pixmap(QCoreApplication::applicationDirPath() + "/assets/NativeStrapper-full-dark.png");
    imageLabel->setPixmap(pixmap.scaled(220, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imageLabel->setAlignment(Qt::AlignCenter);
    rightLayout->addWidget(imageLabel, 0, Qt::AlignHCenter);

    statusLabel = new QLabel("No scatter loaded :(");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet(
        "color: #888888;"
        "font-size: 11px;"
        "padding: 4px 10px;"
        "border-radius: 4px;"
        "background-color: #333333;"
    );
    rightLayout->addWidget(statusLabel, 0, Qt::AlignHCenter);

    layout->addWidget(rightWidget);
    setCentralWidget(root);
}