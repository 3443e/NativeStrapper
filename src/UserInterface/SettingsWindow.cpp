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
#include <QCheckBox>
#include <QTabWidget>
#include <QComboBox>
#include <qapplication.h>
#include <QPainter>

// a horizontal separator line
static QFrame* makeSeparator() {
    auto *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #333333;");
    return sep;
}

static QLabel* makeTitle(const QString &text) {
    auto *label = new QLabel(text);
    label->setStyleSheet("font-size: 15px; font-weight: 465; color: #ffffff;");
    return label;
}

static QLabel* makeDesc(const QString &text) {
    auto *label = new QLabel(text);
    label->setStyleSheet("font-size: 10px; color: #888888;");
    return label;
}

static QWidget* makeToggleRow(const QString &label, const QString &desc, bool defaultVal = false) {
    auto *container = new QWidget();

    auto *outerLayout = new QVBoxLayout(container);
    outerLayout->setContentsMargins(0, 4, 0, 4);

    auto *row = new QFrame();
    row->setStyleSheet(R"(
        QFrame {
            background-color: rgba(255, 255, 255, 0.03);
            border: 1px solid rgba(255, 255, 255, 0.05);
            border-radius: 3px;
        }
    )");

    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(12, 10, 12, 10);

    auto *textCol = new QVBoxLayout();
    textCol->setSpacing(2);

    auto *title = new QLabel(label);
    title->setStyleSheet("color: #cccccc; font-size: 11px; background: transparent; border: none;");

    auto *subtitle = new QLabel(desc);
    subtitle->setStyleSheet("color: #666666; font-size: 9px; background: transparent; border: none;");
    
    textCol->addWidget(title);
    textCol->addWidget(subtitle);

    auto *check = new QCheckBox();
    check->setChecked(defaultVal);

    layout->addLayout(textCol);
    layout->addStretch();
    layout->addWidget(check);

    outerLayout->addWidget(row);

    return container;
}

// a button row with title, description and a button
static QWidget* makeButtonRow(const QString &label, const QString &desc, const QString &btnText, std::function<void()> onClick) {
    auto *row = new QWidget();
    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 4, 0, 4);

    auto *textCol = new QVBoxLayout();
    textCol->setSpacing(2);
    auto *title = new QLabel(label);
    title->setStyleSheet("color: #cccccc; font-size: 11px;");
    auto *subtitle = new QLabel(desc);
    subtitle->setStyleSheet("color: #666666; font-size: 9px;");
    textCol->addWidget(title);
    textCol->addWidget(subtitle);

    auto *btn = new QPushButton(btnText);
    btn->setFixedWidth(100);
    QObject::connect(btn, &QPushButton::clicked, [onClick]() { onClick(); });

    layout->addLayout(textCol);
    layout->addStretch();
    layout->addWidget(btn);

    return row;
}

// a dropdown row with title, description and a combobox
static QWidget* makeDropdownRow(const QString &label, const QString &desc, const QStringList &options) {
    auto *row = new QWidget();
    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 4, 0, 4);

    auto *textCol = new QVBoxLayout();
    textCol->setSpacing(2);
    auto *title = new QLabel(label);
    title->setStyleSheet("color: #cccccc; font-size: 11px;");
    auto *subtitle = new QLabel(desc);
    subtitle->setStyleSheet("color: #666666; font-size: 9px;");
    textCol->addWidget(title);
    textCol->addWidget(subtitle);

    auto *combo = new QComboBox();
    combo->addItems(options);
    combo->setFixedWidth(120);
    combo->setStyleSheet(
        "QComboBox {"
        "  background-color: #2e2e2e;"
        "  color: #cccccc;"
        "  border: 1px solid #555555;"
        "  border-radius: 3px;"
        "  padding: 2px 6px;"
        "  font-size: 10px;"
        "}"
        "QComboBox::drop-down { border: none; width: 16px; }"
        "QComboBox::down-arrow { width: 8px; height: 8px; }"
        "QComboBox QAbstractItemView {"
        "  background-color: #2e2e2e;"
        "  color: #cccccc;"
        "  selection-background-color: #3a3a3a;"
        "  border: 1px solid #555555;"
        "}"
    );

    layout->addLayout(textCol);
    layout->addStretch();
    layout->addWidget(combo);

    return row;
}

static QWidget* makeBootstrapperPage() {
    auto *page = new QWidget();
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(8);

    layout->addWidget(makeTitle("Bootstrapper"));
    layout->addWidget(makeDesc("Configure how NativeStrapper launches Roblox."));
    layout->addWidget(makeSeparator());

    layout->addWidget(makeToggleRow(
        "Prompt before launching another instance",
        "Show a confirmation dialog before opening a second Roblox instance.",
        false
    ));

    layout->addStretch();
    return page;
}

static QWidget* makeDeploymentPage() {
    auto *page = new QWidget();
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(8);

    layout->addWidget(makeTitle("Deployment"));
    layout->addWidget(makeDesc("Manage Roblox updates and installation."));
    layout->addWidget(makeSeparator());

    layout->addWidget(makeDropdownRow(
        "Channel",
        "The Roblox deployment channel to use.",
        {"production", "live", "zlive"}
    ));

    layout->addWidget(makeToggleRow(
        "Enable Roblox updates",
        "Automatically update Roblox when a new version is available.",
        true
    ));

    layout->addWidget(makeToggleRow(
        "Enable NativeStrapper updates",
        "Automatically update NativeStrapper when a new version is available.",
        true
    ));

    layout->addWidget(makeToggleRow(
        "Enable NativeStrapper updates",
        "Automatically update NativeStrapper when a new version is available.",
        true
    ));

    layout->addStretch();
    return page;
}

static QWidget* makeIntegrationsPage() {
    auto *page = new QWidget();
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(8);

    layout->addWidget(makeTitle("Integrations"));
    layout->addWidget(makeDesc("Manage third-party integrations."));
    layout->addWidget(makeSeparator());

    layout->addWidget(makeToggleRow(
        "Enable activity tracker",
        "Tracks which Roblox game you are currently playing while NativeStrap is running through log files.",
        false
    ));

    layout->addWidget(makeTitle("Discord Rich Presence"));
    layout->addWidget(makeDesc("Discord Rich Presence needs activity tracking to work!"));
    layout->addWidget(makeSeparator());

    layout->addWidget(makeToggleRow(
        "Enable Discord RPC",
        "Show your current Roblox game in Discord.",
        false
    ));
    
    layout->addWidget(makeToggleRow(
        "Show game name",
        "Display the name of the game you are playing.",
        true
    ));
    layout->addWidget(makeToggleRow(
        "Join button",
        "idk",
        true
    ));
    layout->addStretch();

    // custom integrations tab
    auto *customTab = new QWidget();
    auto *customLayout = new QVBoxLayout(customTab);
    customLayout->setContentsMargins(12, 12, 12, 12);
    customLayout->setSpacing(8);
    customLayout->addWidget(makeToggleRow(
        "Run custom commands on launch",
        "Execute custom commands when Roblox starts.",
        false
    ));
    customLayout->addWidget(makeButtonRow(
        "Custom commands",
        "Add commands to run when Roblox launches or exits.",
        "Edit",
        []() {
            // TODO
        }
    ));
    customLayout->addStretch();
    //tabs->addTab(customTab, "Custom");

    layout->addStretch();
    return page;
}

static QIcon coloredIcon(const QString &path, const QColor &color) {
    QPixmap pixmap(path);
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    painter.end();
    return QIcon(pixmap);
}

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

    auto *appdirBtn = new QPushButton();
    appdirBtn->setFixedWidth(200);
    appdirBtn->setFixedHeight(24);
    appdirBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #2e2e2e;"
        "  color: #cccccc;"
        "  border: 1px solid #555555;"
        "  border-radius: 3px;"
        "  padding: 2px 6px 2px 10px;"
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

    auto *appdirMenu = new QMenu(appdirBtn);
    appdirMenu->setStyleSheet(
        "QMenu {"
        "  background-color: #2e2e2e;"
        "  color: #cccccc;"
        "  border: 1px solid #555555;"
        "  font-size: 10px;"
        "}"
        "QMenu::item { padding: 4px 16px; }"
        "QMenu::item:selected { background-color: #3a3a3a; }"
        "QMenu::indicator { width: 12px; height: 12px; }"
    );

    auto updateBtnText = [=]() {
        QStringList selected;
        for (auto *action : appdirMenu->actions()) {
            if (action->isChecked()) {
                selected << action->text();
            }
        }
        appdirBtn->setText("  " + (selected.isEmpty() ? "none" : selected.join(", ")));
    };

    if (script.appdirectories.empty()) {
        appdirBtn->setText("  No directories defined");
        appdirBtn->setEnabled(false);
    } else if (script.appdirectories.size() == 1) {
        appdirBtn->setText("  " + QString::fromStdString(script.appdirectories[0].label));
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
        "  padding: 8px 10px 8px 10px;"
        "  font-size: 11px;"
        "  text-align: left;"
        "}"
        "QPushButton:hover { background-color: #2a2a2a; color: #dddddd; }"
        "QPushButton:checked { background-color: #2e2e2e; color: #ffffff; }";

    auto *bootstrapperBtn = new QPushButton("Bootstrapper");
    bootstrapperBtn->setStyleSheet(sidebarBtnStyle);
    bootstrapperBtn->setCheckable(true);
    bootstrapperBtn->setIcon(coloredIcon(QCoreApplication::applicationDirPath() + "/assets/svgs/strikethrough.svg", QColor("#aaaaaa")));
    bootstrapperBtn->setIconSize(QSize(16, 16));

    auto *deploymentBtn = new QPushButton("Deployment");
    deploymentBtn->setStyleSheet(sidebarBtnStyle);
    deploymentBtn->setCheckable(true);
    deploymentBtn->setIcon(coloredIcon(QCoreApplication::applicationDirPath() + "/assets/svgs/deployment.svg", QColor("#aaaaaa")));
    deploymentBtn->setIconSize(QSize(16, 16));
    
    auto *integrationsBtn = new QPushButton("Integrations");
    integrationsBtn->setStyleSheet(sidebarBtnStyle);
    integrationsBtn->setCheckable(true);
    integrationsBtn->setChecked(true); // since it's the first
    integrationsBtn->setIcon(coloredIcon(QCoreApplication::applicationDirPath() + "/assets/svgs/integrations.svg", QColor("#aaaaaa")));
    integrationsBtn->setIconSize(QSize(16, 16));

    sidebarLayout->addWidget(integrationsBtn);
    sidebarLayout->addWidget(bootstrapperBtn);
    sidebarLayout->addWidget(deploymentBtn);

    sidebarLayout->addStretch();
    mainLayout->addWidget(sidebar);

    auto *stack = new QStackedWidget();
    stack->addWidget(makeIntegrationsPage());
    stack->addWidget(makeBootstrapperPage());
    stack->addWidget(makeDeploymentPage());
    stack->setCurrentIndex(0);

    mainLayout->addWidget(stack);
    rootLayout->addWidget(mainArea);

    auto deselectAll = [=]() {
        bootstrapperBtn->setChecked(false);
        deploymentBtn->setChecked(false);
        integrationsBtn->setChecked(false);
    };

    QObject::connect(integrationsBtn, &QPushButton::clicked, [=]() {
        deselectAll(); integrationsBtn->setChecked(true); stack->setCurrentIndex(0);
    });
    QObject::connect(bootstrapperBtn, &QPushButton::clicked, [=]() {
        deselectAll(); bootstrapperBtn->setChecked(true); stack->setCurrentIndex(1);
    });
    QObject::connect(deploymentBtn, &QPushButton::clicked, [=]() {
        deselectAll(); deploymentBtn->setChecked(true); stack->setCurrentIndex(2);
    });

    setCentralWidget(root);
}