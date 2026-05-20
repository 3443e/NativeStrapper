#include "UserInterface/OnboardingWindow.hpp"
#include "UserInterface/SettingsWindow.hpp"
#include "UserInterface/VesselInfoWindow.hpp"
#include "VesselManager.hpp"
#include <QCoreApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QListWidget>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

OnboardingWindow::OnboardingWindow() {
    setWindowTitle("NativeStrapper");
    setFixedSize(400, 240);

    auto *root = new QWidget();
    auto *layout = new QHBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *leftWidget = new QWidget();
    leftWidget->setObjectName("leftWidget");
    leftWidget->setStyleSheet("QWidget#leftWidget { background-color: #1a1a1a; }");
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
    rightLayout->setContentsMargins(0, 20, 0, 8);
    rightLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QLabel *imageLabel = new QLabel();
    QPixmap pixmap(QCoreApplication::applicationDirPath() + "/assets/NativeStrapper-full-dark.png");
    imageLabel->setPixmap(pixmap.scaled(220, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imageLabel->setAlignment(Qt::AlignCenter);
    rightLayout->addWidget(imageLabel, 0, Qt::AlignHCenter);

    QLabel *quoteLabel = new QLabel("A cross-platform roblox bootstrapper.");
    quoteLabel->setAlignment(Qt::AlignCenter);
    quoteLabel->setStyleSheet("color: #bebebe; font-size: 9px;");
    rightLayout->addWidget(quoteLabel, 0, Qt::AlignHCenter);

    rightLayout->addSpacing(6);

    QLabel *vesselTitleLabel = new QLabel("Loaded Vessels:");
    vesselTitleLabel->setAlignment(Qt::AlignCenter);
    vesselTitleLabel->setStyleSheet("color: #ffffff; font-size: 10px;");
    rightLayout->addWidget(vesselTitleLabel, 0, Qt::AlignHCenter);

    rightLayout->addSpacing(2);

    QWidget *statusContainer = new QWidget();
    statusContainer->setFixedSize(200, 60);
    QVBoxLayout *statusLayout = new QVBoxLayout(statusContainer);
    statusLayout->setContentsMargins(0, 0, 0, 0);
    statusLayout->setAlignment(Qt::AlignCenter);

    statusLabel = new QLabel("No vessels loaded :(");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet(
        "color: #888888;"
        "font-size: 11px;"
        "padding: 4px 10px;"
        "border-radius: 4px;"
        "background-color: #333333;"
    );
    statusLayout->addWidget(statusLabel);

    vesselList = new QListWidget();
    vesselList->setStyleSheet(
        "QListWidget {"
        "  background-color: #333333;"
        "  color: #cccccc;"
        "  border-radius: 4px;"
        "  font-size: 11px;"
        "  padding: 2px;"
        "}"
        "QListWidget::item { padding: 2px 6px; }"
        "QListWidget::item:selected { background-color: #444444; }"
    );
    statusLayout->addWidget(vesselList);

    rightLayout->addWidget(statusContainer, 0, Qt::AlignHCenter);
    rightLayout->addStretch();

    QString btnStyle =
        "QPushButton {"
        "  background-color: #3d3d3d;"
        "  color: #dddddd;"
        "  border: 1px solid #555555;"
        "  border-radius: 3px;"
        "  padding: 3px 6px;"
        "  font-size: 10px;"
        "}"
        "QPushButton:hover { background-color: #4a4a4a; }"
        "QPushButton:pressed { background-color: #2e2e2e; }"
        "QPushButton:disabled {"
        "  background-color: #2e2e2e;"
        "  color: #555555;"
        "  border: 1px solid #3a3a3a;"
        "}";

    QWidget *btnRow = new QWidget();
    QHBoxLayout *btnLayout = new QHBoxLayout(btnRow);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(4);

    QPushButton *importBtn = new QPushButton("Import");
    importBtn->setStyleSheet(btnStyle);
    QObject::connect(importBtn, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getOpenFileName(
            this, "Import Vessel", QString(),
            "Vessel Files (*.json);;All Files (*)"
        );
        if (path.isEmpty()) return;

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

        QTextStream in(&file);
        std::string contents = in.readAll().toStdString();
        file.close();

        VesselManager::Vessel *vessel = VesselManager::ParseVessel(contents);
        if (!vessel) return;

        QMessageBox warn(this);
        warn.setWindowTitle("Import Vessel");
        warn.setText(QString(
            "You are about to install \"%1\".\n\n"
            "Only import vessels from sources you trust. "
            "A vessel can run arbitrary commands on your system.\n\n"
            "This will also register it as your Roblox URI handler."
        ).arg(QString::fromStdString(vessel->VesselTitle)));
        warn.setIcon(QMessageBox::Warning);
        warn.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        if (warn.exec() != QMessageBox::Ok) {
            delete vessel;
            return;
        }

        auto result = VesselManager::InstallVessel(vessel);
        if (result == VesselManager::VesselInstallResult::VINSTALLUNKNOWNENVIRONMENT) {
            QMessageBox::critical(this, "Error", "Installing vessels is not supported on this platform.");
            delete vessel;
            return;
        }

        delete vessel;
        refreshVesselView();
    });

    viewBtn = new QPushButton("View");
    viewBtn->setStyleSheet(btnStyle);
    viewBtn->setEnabled(false);
    QObject::connect(viewBtn, &QPushButton::clicked, [this]() {
        int row = vesselList->currentRow();
        if (row < 0 || row >= (int)VesselManager::LoadedVessels.size()) return;

        VesselInfoWindow *w = new VesselInfoWindow(VesselManager::LoadedVessels[row], this);
        w->exec();
    });

    uninstallBtn = new QPushButton("Uninstall");
    uninstallBtn->setStyleSheet(btnStyle);
    uninstallBtn->setEnabled(false);
    QObject::connect(uninstallBtn, &QPushButton::clicked, [this]() {
        int row = vesselList->currentRow();
        if (row < 0 || row >= (int)VesselManager::LoadedVessels.size()) return;

        VesselManager::Vessel &vessel = VesselManager::LoadedVessels[row];

        QMessageBox warn(this);
        warn.setWindowTitle("Uninstall Vessel");
        warn.setText(QString(
            "Are you sure you want to uninstall \"%1\"?\n\n"
            "This will remove it as your Roblox URI handler."
        ).arg(QString::fromStdString(vessel.VesselTitle)));
        warn.setIcon(QMessageBox::Warning);
        warn.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        if (warn.exec() != QMessageBox::Ok) return;

        VesselManager::UninstallVessel(&vessel);
        viewBtn->setEnabled(false);
        uninstallBtn->setEnabled(false);
        refreshVesselView();
    });

    QObject::connect(vesselList, &QListWidget::itemSelectionChanged, [this]() {
        bool selected = vesselList->currentRow() >= 0;
        viewBtn->setEnabled(selected);
        uninstallBtn->setEnabled(selected);
    });

    btnLayout->addWidget(importBtn);
    btnLayout->addWidget(viewBtn);
    btnLayout->addWidget(uninstallBtn);

    rightLayout->addWidget(btnRow, 0, Qt::AlignHCenter);

    layout->addWidget(rightWidget);
    setCentralWidget(root);

    refreshVesselView();
}

void OnboardingWindow::refreshVesselView() {
    bool hasVessels = !VesselManager::LoadedVessels.empty();

    statusLabel->setVisible(!hasVessels);
    vesselList->setVisible(hasVessels);

    if (hasVessels) {
        int prevRow = vesselList->currentRow();
        vesselList->clear();
        for (const auto &vessel : VesselManager::LoadedVessels) {
            vesselList->addItem(QString::fromStdString(vessel.VesselTitle));
        }
        if (prevRow >= 0 && prevRow < vesselList->count()) {
            vesselList->setCurrentRow(prevRow);
        }
    }
}