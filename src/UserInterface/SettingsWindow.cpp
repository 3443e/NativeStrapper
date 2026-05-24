#include "UserInterface/SettingsWindow.hpp"
#include "BootstrapScripts/ScriptManager.hpp"
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QFrame>
#include <QMenu>
#include <QAction>

SettingsWindow::SettingsWindow(const ScriptManager::BootstrapScript &script) {
    setWindowTitle("NativeStrapper - Settings");
    setFixedSize(800, 500);

    auto *root = new QWidget();
    auto *rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // top bar
    auto *topBar = new QWidget();
    topBar->setObjectName("topBar");
    topBar->setStyleSheet("QWidget#topBar { background-color: #1e1e1e; border-bottom: 1px solid #333333; }");
    topBar->setFixedHeight(40);
    auto *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(12, 0, 12, 0);
    topLayout->setSpacing(8);

    auto *topLabel = new QLabel("Modifying:");
    topLabel->setStyleSheet("color: #888888; font-size: 10px;");
    topLayout->addWidget(topLabel);

    // dropdown button
    auto *appdirBtn = new QPushButton();
    appdirBtn->setFixedWidth(200);
    appdirBtn->setFixedHeight(24);
    appdirBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #2e2e2e;"
        "  color: #cccccc;"
        "  border: 1px solid #555555;"
        "  border-radius: 3px;"
        "  padding: 2px 300px;"
        "  font-size: 10px;"
        "  text-align: left;"
        "}"
        "QPushButton:hover { background-color: #3a3a3a; }"
        "QPushButton::menu-indicator {"
        "  width: 8px;"
        "  height: 8px;"
        "  right: 6px;"
        "  bottom: 7px;"
        "}"
    );

    // build the menu with checkable actions
    auto *appdirMenu = new QMenu(appdirBtn);
    appdirMenu->setStyleSheet(
        "QMenu {"
        "  background-color: #2e2e2e;"
        "  color: #cccccc;"
        "  border: 1px solid #555555;"
        "  font-size: 10px;"
        "}"
        "QMenu::item { padding: 4px 16px;}"
        "QMenu::item:selected { background-color: #3a3a3a }"
        "QMenu::indicator { width: 12px; height: 12px; }"
    );

    auto updateBtnText = [=]() {
        QStringList selected;
        for (auto *action : appdirMenu->actions())
            if (action->isChecked())
                selected << action->text();
        appdirBtn->setText(selected.isEmpty() ? "none" : selected.join(", "));
    };

    if (script.appdirectories.empty()) {
        appdirBtn->setText("No directories defined");
        appdirBtn->setEnabled(false);
    } else if (script.appdirectories.size() == 1) {
        appdirBtn->setText(QString::fromStdString(script.appdirectories[0].label));
        appdirBtn->setEnabled(false);
    } else {
        for (const auto &dir : script.appdirectories) {
            auto *action = new QAction(QString::fromStdString(dir.label), appdirMenu);
            action->setCheckable(true);
            action->setChecked(true);
            QObject::connect(action, &QAction::toggled, [=]() {
                updateBtnText();
            });
            appdirMenu->addAction(action);
        }
        appdirBtn->setMenu(appdirMenu);
        updateBtnText();
    }

    topLayout->addWidget(appdirBtn);
    topLayout->addStretch();
    rootLayout->addWidget(topBar);

    // main area
    auto *mainArea = new QWidget();
    auto *mainLayout = new QHBoxLayout(mainArea);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // sidebar
    auto *sidebar = new QWidget();
    sidebar->setObjectName("sidebar");
    sidebar->setStyleSheet("QWidget#sidebar { background-color: #1a1a1a; }");
    sidebar->setFixedWidth(160);
    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(6, 10, 6, 10);
    sidebarLayout->setSpacing(4);

    QString sidebarBtnStyle =
        "QPushButton {"
        "  background-color: transparent;"
        "  color: #aaaaaa;"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 8px 10px;"
        "  font-size: 11px;"
        "  text-align: left;"
        "}"
        "QPushButton:hover { background-color: #2a2a2a; color: #dddddd; }"
        "QPushButton:checked { background-color: #2e2e2e; color: #ffffff; }";

    QPushButton *bootstrapperBtn = new QPushButton("Bootstrapper");
    bootstrapperBtn->setStyleSheet(sidebarBtnStyle);
    bootstrapperBtn->setCheckable(true);
    bootstrapperBtn->setChecked(true);
    sidebarLayout->addWidget(bootstrapperBtn);

    QPushButton *integrationsBtn = new QPushButton("Integrations");
    integrationsBtn->setStyleSheet(sidebarBtnStyle);
    integrationsBtn->setCheckable(true);
    sidebarLayout->addWidget(integrationsBtn);

    sidebarLayout->addStretch();
    mainLayout->addWidget(sidebar);

    // content stack
    auto *stack = new QStackedWidget();

    // bootstrapper page
    auto *bootstrapperPage = new QWidget();
    auto *bootstrapperLayout = new QVBoxLayout(bootstrapperPage);
    bootstrapperLayout->setContentsMargins(20, 20, 20, 20);
    bootstrapperLayout->setSpacing(12);

    auto *bTitle = new QLabel("Bootstrapper");
    bTitle->setStyleSheet("font-size: 14px; color: #ffffff;");
    bootstrapperLayout->addWidget(bTitle);

    auto *bDesc = new QLabel("Configure how NativeStrapper launches Roblox.");
    bDesc->setStyleSheet("font-size: 10px; color: #888888;");
    bootstrapperLayout->addWidget(bDesc);

    auto *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("color: #333333;");
    bootstrapperLayout->addWidget(separator);

    bootstrapperLayout->addWidget(new QLabel("More settings coming soon..."));
    bootstrapperLayout->addStretch();
    stack->addWidget(bootstrapperPage);

    // integrations page
    auto *integrationsPage = new QWidget();
    auto *integrationsLayout = new QVBoxLayout(integrationsPage);
    integrationsLayout->setContentsMargins(20, 20, 20, 20);
    integrationsLayout->setSpacing(12);

    auto *iTitle = new QLabel("Integrations");
    iTitle->setStyleSheet("font-size: 14px; color: #ffffff;");
    integrationsLayout->addWidget(iTitle);

    auto *iDesc = new QLabel("Manage third-party integrations.");
    iDesc->setStyleSheet("font-size: 10px; color: #888888;");
    integrationsLayout->addWidget(iDesc);

    auto *separator2 = new QFrame();
    separator2->setFrameShape(QFrame::HLine);
    separator2->setStyleSheet("color: #333333;");
    integrationsLayout->addWidget(separator2);

    integrationsLayout->addWidget(new QLabel("No integrations available yet."));
    integrationsLayout->addStretch();
    stack->addWidget(integrationsPage);

    mainLayout->addWidget(stack);
    rootLayout->addWidget(mainArea);

    QObject::connect(bootstrapperBtn, &QPushButton::clicked, [=]() {
        stack->setCurrentIndex(0);
        bootstrapperBtn->setChecked(true);
        integrationsBtn->setChecked(false);
    });

    QObject::connect(integrationsBtn, &QPushButton::clicked, [=]() {
        stack->setCurrentIndex(1);
        integrationsBtn->setChecked(true);
        bootstrapperBtn->setChecked(false);
    });

    setCentralWidget(root);
}