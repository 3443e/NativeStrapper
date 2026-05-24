#include "UserInterface/OnboardingWindow.hpp"
#include "UserInterface/SettingsWindow.hpp"
#include "BootstrapScripts/ScriptManager.hpp"
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
    launchBtn = new QPushButton("Launch Roblox");
    launchBtn->setEnabled(false);
    QObject::connect(launchBtn, &QPushButton::clicked, [this]() {
        int row = scriptList->currentRow();
        if (row < 0 || row >= (int)ScriptManager::LoadedScripts.size()) return;
        // TODO: launch roblox with selected script
    });
    left->addWidget(launchBtn);

    settingsBtn = new QPushButton("Settings", leftWidget);
    settingsBtn->setEnabled(false);
    QObject::connect(settingsBtn, &QPushButton::clicked, [this]() {
        int row = scriptList->currentRow();
        if (row < 0 || row >= (int)ScriptManager::LoadedScripts.size()) return;
        SettingsWindow *window = new SettingsWindow(ScriptManager::LoadedScripts[row]);
        window->show();
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

    QLabel *scriptsLabel = new QLabel("Bootstrap Scripts:");
    scriptsLabel->setAlignment(Qt::AlignCenter);
    scriptsLabel->setStyleSheet("color: #ffffff; font-size: 10px;");
    rightLayout->addWidget(scriptsLabel, 0, Qt::AlignHCenter);

    rightLayout->addSpacing(2);

    QWidget *statusContainer = new QWidget();
    statusContainer->setFixedSize(200, 60);
    QVBoxLayout *statusLayout = new QVBoxLayout(statusContainer);
    statusLayout->setContentsMargins(0, 0, 0, 0);
    statusLayout->setAlignment(Qt::AlignCenter);

    statusLabel = new QLabel("No scripts loaded :(");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet(
        "color: #888888;"
        "font-size: 11px;"
        "padding: 4px 10px;"
        "border-radius: 4px;"
        "background-color: #333333;"
    );
    statusLayout->addWidget(statusLabel);

    scriptList = new QListWidget();
    scriptList->setStyleSheet(
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
    statusLayout->addWidget(scriptList);

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

    QPushButton *importBtn = new QPushButton("Import Script");
    importBtn->setStyleSheet(btnStyle);
    QObject::connect(importBtn, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getOpenFileName(
            this, "Import Bootstrap Script", QString(),
            "Bootstrap Scripts (*.lua);;All Files (*)"
        );
        if (path.isEmpty()) return;

        // warn the user
        QMessageBox warn(this);
        warn.setWindowTitle("Import Bootstrap Script");
        warn.setText(QString(
            "You are about to install \"%1\".\n\n"
            "Only import scripts from sources you trust. "
            "A bootstrap script can run arbitrary commands on your system."
        ).arg(QString::fromStdString(path.toStdString())));
        warn.setIcon(QMessageBox::Warning);
        warn.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        if (warn.exec() != QMessageBox::Ok) return;

        // read metadata from the lua file first (loads the script)
        ScriptManager::BootstrapScript info;
        if (!ScriptManager::ReadMetadata(path.toStdString(), info)) {
            QMessageBox::critical(this, "Error", "Failed to read script metadata. Make sure it has a valid metadata table.");
            return;
        }

        if (!ScriptManager::InstallScript(path.toStdString())) {
            QMessageBox::critical(this, "Error", "Failed to install script.");
            return;
        }

        refreshScriptView();
    });

    viewBtn = new QPushButton("View");
    viewBtn->setStyleSheet(btnStyle);
    viewBtn->setEnabled(false);

    uninstallBtn = new QPushButton("Uninstall");
    uninstallBtn->setStyleSheet(btnStyle);
    uninstallBtn->setEnabled(false);
    QObject::connect(uninstallBtn, &QPushButton::clicked, [this]() {
        int row = scriptList->currentRow();
        if (row < 0 || row >= (int)ScriptManager::LoadedScripts.size()) return;

        const std::string title = ScriptManager::LoadedScripts[row].title;

        QMessageBox warn(this);
        warn.setWindowTitle("Uninstall Script");
        warn.setText(QString("Are you sure you want to uninstall \"%1\"?")
            .arg(QString::fromStdString(title)));
        warn.setIcon(QMessageBox::Warning);
        warn.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        if (warn.exec() != QMessageBox::Ok) return;

        ScriptManager::UninstallScript(title);
        viewBtn->setEnabled(false);
        uninstallBtn->setEnabled(false);
        refreshScriptView();
    });

    QObject::connect(scriptList, &QListWidget::itemSelectionChanged, [this]() {
        bool selected = scriptList->currentRow() >= 0;
        viewBtn->setEnabled(selected);
        uninstallBtn->setEnabled(selected);
        launchBtn->setEnabled(selected);
        settingsBtn->setEnabled(selected);
    });

    btnLayout->addWidget(importBtn);
    btnLayout->addWidget(viewBtn);
    btnLayout->addWidget(uninstallBtn);

    rightLayout->addWidget(btnRow, 0, Qt::AlignHCenter);

    layout->addWidget(rightWidget);
    setCentralWidget(root);

    refreshScriptView();
}

void OnboardingWindow::refreshScriptView() {
    bool hasScripts = !ScriptManager::LoadedScripts.empty();

    statusLabel->setVisible(!hasScripts);
    scriptList->setVisible(hasScripts);

    if (hasScripts) {
        int prevRow = scriptList->currentRow();
        scriptList->clear();
        for (const auto &script : ScriptManager::LoadedScripts) {
            scriptList->addItem(QString::fromStdString(script.title));
        }
        if (prevRow >= 0 && prevRow < scriptList->count()) {
            scriptList->setCurrentRow(prevRow);
        }
    }
}