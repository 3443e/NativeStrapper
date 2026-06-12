#include "UserInterface/SettingsWindow.hpp"
#include "BootstrapScripts/ScriptManager.hpp"
#include "TitleBarDarkMode.hpp"
#include "ConfigSaving.hpp"
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
#include <QComboBox>
#include <QCoreApplication>
#include <QPainter>
#include <QScrollArea>
#include <QEvent>
#include <QDialog>
#include <QListWidget>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

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

class TagBadge : public QWidget {
public:
    TagBadge(const QString &hoverText, QWidget *parent = nullptr)  : QWidget(parent), m_hoverText(hoverText) {
        setFixedSize(13, 13);
        setMouseTracking(true);

        m_pill = new QLabel(hoverText);
        m_pill->setStyleSheet(
            "color: #888888;"
            "background-color: #252525;"
            "border: 1px solid #333333;"
            "border-radius: 2px;"
            "font-size: 8px;"
            "font-weight: 400;"
            "padding: 1px 5px;"
        );
        m_pill->adjustSize();
        m_pill->hide();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        if (m_hovered) {
            p.setPen(QPen(QColor(255, 255, 255, 40), 1));
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(rect().adjusted(1, 1, -1, -1));
        }
        p.setPen(QColor(255, 255, 255, m_hovered ? 90 : 30));
        QFont f = p.font();
        f.setPixelSize(7);
        f.setBold(false);
        p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, "i");
    }

    void enterEvent(QEvent *) override {
        m_hovered = true;
        update();
        QWidget *win = window();
        if (!win) return;
        QPoint pos = mapTo(win, QPoint(width() + 5, (height() - m_pill->height()) / 2));
        m_pill->setParent(win);
        m_pill->move(pos);
        m_pill->raise();
        m_pill->show();
    }

    void leaveEvent(QEvent *) override {
        m_hovered = false;
        update();
        m_pill->hide();
    }

private:
    QString m_hoverText;
    QLabel *m_pill    = nullptr;
    bool m_hovered = false;
};

static QWidget* makeToggleRow(const QString &label, const QString &desc, const ScriptManager::BootstrapScript &script, const std::string &key, bool defaultVal = false, const QString &badgeHover = "INTEGRATED") {
    auto *container = new QWidget();
    auto *outerLayout = new QVBoxLayout(container);
    outerLayout->setContentsMargins(0, 4, 0, 4);

    auto *row = new QFrame();
    row->setStyleSheet(
        "QFrame {"
        "  background-color: rgba(255, 255, 255, 0.03);"
        "  border: 1px solid rgba(255, 255, 255, 0.05);"
        "  border-radius: 3px;"
        "}"
    );

    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(12, 10, 12, 10);

    auto *textCol = new QVBoxLayout();
    textCol->setSpacing(2);

    auto *labelRow = new QHBoxLayout();
    labelRow->setSpacing(5);
    labelRow->setContentsMargins(0, 0, 0, 0);

    auto *titleLabel = new QLabel(label);
    titleLabel->setStyleSheet("color: #cccccc; font-size: 11px; background: transparent; border: none;");

    auto *badge = new TagBadge(badgeHover);

    labelRow->addWidget(titleLabel);
    labelRow->addWidget(badge);
    labelRow->addStretch();

    auto *descLabel = new QLabel(desc);
    descLabel->setStyleSheet("color: #666666; font-size: 9px; background: transparent; border: none;");

    textCol->addLayout(labelRow);
    textCol->addWidget(descLabel);

    auto *check = new QCheckBox();
    check->setChecked(ConfigSaving::GetScriptSettingBool(script.title, key, defaultVal));

    QObject::connect(check, &QCheckBox::toggled, [script, key](bool checked) {
        ConfigSaving::SetScriptSetting(script.title, key, checked ? "true" : "false");
    });

    layout->addLayout(textCol);
    layout->addStretch();
    layout->addWidget(check);

    outerLayout->addWidget(row);
    return container;
}

static QWidget* makeButtonRow(const QString &label, const QString &desc, const QString &btnText, std::function<void()> onClick) {
    auto *row = new QWidget();
    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 4, 0, 4);

    auto *textCol = new QVBoxLayout();
    textCol->setSpacing(2);
    auto *titleLabel = new QLabel(label);
    titleLabel->setStyleSheet("color: #cccccc; font-size: 11px;");
    auto *descLabel = new QLabel(desc);
    descLabel->setStyleSheet("color: #666666; font-size: 9px;");
    textCol->addWidget(titleLabel);
    textCol->addWidget(descLabel);

    auto *btn = new QPushButton(btnText);
    btn->setFixedWidth(100);
    QObject::connect(btn, &QPushButton::clicked, [onClick]() { onClick(); });

    layout->addLayout(textCol);
    layout->addStretch();
    layout->addWidget(btn);
    return row;
}

static QWidget* makeDropdownRow(const QString &label, const QString &desc, const QStringList &options, const ScriptManager::BootstrapScript &script, const std::string &key, const QString &badgeHover = "INTEGRATED") {
    auto *row = new QWidget();
    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 4, 0, 4);

    auto *textCol = new QVBoxLayout();
    textCol->setSpacing(2);

    auto *labelRow = new QHBoxLayout();
    labelRow->setSpacing(5);
    labelRow->setContentsMargins(0, 0, 0, 0);

    auto *titleLabel = new QLabel(label);
    titleLabel->setStyleSheet("color: #cccccc; font-size: 11px;");

    auto *badge = new TagBadge(badgeHover);

    labelRow->addWidget(titleLabel);
    labelRow->addWidget(badge);
    labelRow->addStretch();

    auto *descLabel = new QLabel(desc);
    descLabel->setStyleSheet("color: #666666; font-size: 9px;");

    textCol->addLayout(labelRow);
    textCol->addWidget(descLabel);

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

    std::string saved = ConfigSaving::GetScriptSetting(script.title, key, options[0].toStdString());
    int idx = options.indexOf(QString::fromStdString(saved));
    if (idx >= 0) combo->setCurrentIndex(idx);

    QObject::connect(combo, &QComboBox::currentTextChanged, [script, key](const QString &text) {
        ConfigSaving::SetScriptSetting(script.title, key, text.toStdString());
    });

    layout->addLayout(textCol);
    layout->addStretch();
    layout->addWidget(combo);
    return row;
}

static QIcon coloredIcon(const QString &path, const QColor &color) {
    QPixmap pixmap(path);
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    painter.end();
    return QIcon(pixmap);
}

static QScrollArea* makeScrollArea(QWidget *content) {
    auto *scroll = new QScrollArea();
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setStyleSheet(
        "QScrollArea { background-color: transparent; border: none; }"
        "QScrollBar:vertical {"
        "  background: #1e1e1e;"
        "  width: 6px;"
        "  margin: 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #444444;"
        "  border-radius: 3px;"
        "  min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover { background: #555555; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
    );
    return scroll;
}

static void openCommandsDialog(const ScriptManager::BootstrapScript &script) {
    auto *dialog = new QDialog();
    dialog->setWindowTitle("Custom Commands");
    dialog->setFixedSize(420, 320);

    auto *layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);

    auto *list = new QListWidget();
    list->setStyleSheet(
        "QListWidget {"
        "  background-color: #1e1e1e;"
        "  color: #cccccc;"
        "  border: 1px solid #333333;"
        "  border-radius: 3px;"
        "  font-size: 11px;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #2e2e2e;"
        "  color: #ffffff;"
        "}"
    );

    // load saved commands
    std::string saved = ConfigSaving::GetScriptSetting(script.title, "customcommands", "");
    if (!saved.empty()) {
        for (auto &cmd : QString::fromStdString(saved).split("\n", Qt::SkipEmptyParts)) {
            list->addItem(cmd);
        }
    }

    auto save = [&]() {
        QStringList cmds;
        for (int i = 0; i < list->count(); i++) {
            cmds << list->item(i)->text();
        }
        ConfigSaving::SetScriptSetting(script.title, "customcommands", cmds.join("\n").toStdString());
    };

    // buttons
    auto *btnRow = new QHBoxLayout();

    auto *addBtn = new QPushButton("Add");
    auto *modifyBtn = new QPushButton("Modify");
    auto *removeBtn = new QPushButton("Remove");

    btnRow->addWidget(addBtn);
    btnRow->addWidget(modifyBtn);
    btnRow->addWidget(removeBtn);
    btnRow->addStretch();

    QObject::connect(addBtn, &QPushButton::clicked, [=, &save]() {
        bool ok;
        QString cmd = QInputDialog::getText(dialog, "Add Command", "Command:", QLineEdit::Normal, "", &ok);
        if (ok && !cmd.trimmed().isEmpty()) {
            list->addItem(cmd.trimmed());
            save();
        }
    });

    QObject::connect(modifyBtn, &QPushButton::clicked, [=, &save]() {
        auto *item = list->currentItem();
        if (!item) return;
        bool ok;
        QString cmd = QInputDialog::getText(dialog, "Modify Command", "Command:", QLineEdit::Normal, item->text(), &ok);
        if (ok && !cmd.trimmed().isEmpty()) {
            item->setText(cmd.trimmed());
            save();
        }
    });

    QObject::connect(removeBtn, &QPushButton::clicked, [=, &save]() {
        auto *item = list->currentItem();
        if (!item) return;
        delete item;
        save();
    });

    layout->addWidget(list);
    layout->addLayout(btnRow);

    dialog->exec();
}

static QWidget* makeNativeStrapperPage(const ScriptManager::BootstrapScript &script) {
    auto *content = new QWidget();
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(8);

    layout->addWidget(makeTitle("NativeStrapper"));
    layout->addWidget(makeDesc("Configure how NativeStrapper launches Roblox."));
    layout->addWidget(makeSeparator());
    layout->addWidget(makeToggleRow(
        "Prompt before launching another instance",
        "Show a confirmation dialog before opening a second Roblox instance. (We recommend you enable this)",
        script, "promptbeforerelaunch", true
    ));
    layout->addStretch();

    return makeScrollArea(content);
}

static QWidget* makeDeploymentPage(const ScriptManager::BootstrapScript &script) {
    auto *content = new QWidget();
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(8);

    layout->addWidget(makeTitle("Deployment"));
    layout->addWidget(makeDesc("Manage Roblox updates and installation."));
    layout->addWidget(makeSeparator());
    layout->addWidget(makeDropdownRow(
        "Channel", "The Roblox deployment channel to use.",
        {"production", "live", "zlive"}, script, "channel"
    ));
    layout->addWidget(makeToggleRow(
        "Enable Roblox updates",
        "Automatically update Roblox when a new version is available.",
        script, "autorobloxupdate", true
    ));
    layout->addWidget(makeToggleRow(
        "Enable NativeStrapper updates",
        "Automatically update NativeStrapper when a new version is available.",
        script, "autonativestrapperupdate", true
    ));
    layout->addWidget(makeToggleRow(
        "Enable bootstrap script updates",
        "Automatically update bootstrap scripts when a new version is available.",
        script, "autobootstrapscriptupdate", true
    ));
    layout->addStretch();

    return makeScrollArea(content);
}


static QWidget* makeIntegrationsPage(const ScriptManager::BootstrapScript &script) {
    auto *content = new QWidget();
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(8);
    layout->addWidget(makeTitle("Integrations"));
    layout->addWidget(makeDesc("Manage third-party integrations."));
    if (ScriptManager::HasCapability(script, "ACTIVITY_WATCHER")) {
        layout->addWidget(makeSeparator());
        layout->addWidget(makeToggleRow(
            "Enable activity tracker",
            "Tracks which Roblox game you are currently playing through log files. (Might break if roblox crashes)",
            script, "enableactivitytracker", false
        ));
        if (ScriptManager::HasCapability(script, "DISCORDRPC")) {
            layout->addSpacing(8);
            layout->addWidget(makeTitle("Discord Rich Presence"));
            layout->addWidget(makeDesc("Discord Rich Presence needs activity tracking to work!"));
            layout->addWidget(makeSeparator());
            layout->addWidget(makeToggleRow(
                "Enable Discord RPC",
                "Show your current Roblox game in Discord.",
                script, "enablediscordrpc", false
            ));
            layout->addWidget(makeToggleRow(
                "Show game name",
                "Display the name of the game you are playing.",
                script, "showdiscordrpcgamename", true
            ));
        }
    }
    layout->addSpacing(8);
    layout->addWidget(makeTitle("Custom"));
    layout->addWidget(makeSeparator());
    layout->addWidget(makeToggleRow(
        "Run custom commands on launch",
        "Execute custom commands when Roblox starts.",
        script, "runcustomcommandsbeforelaunch", false
    ));
    layout->addWidget(makeButtonRow(
        "Custom commands",
        "Add commands to run when Roblox launches or exits.",
        "Edit", [script]() { openCommandsDialog(script); }
    ));
    layout->addStretch();
    return makeScrollArea(content);
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
    topLayout->setSpacing(6);

    auto *topLabel = new QLabel("Modifying:");
    topLabel->setStyleSheet("color: #444444; font-size: 9px; font-weight: 300;");
    topLayout->addWidget(topLabel);

    auto *scriptNameLabel = new QLabel(QString::fromStdString(script.title));
    scriptNameLabel->setStyleSheet("color: #888888; font-size: 9px; font-weight: 300;");
    topLayout->addWidget(scriptNameLabel);

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

    QString iconDir = QCoreApplication::applicationDirPath() + "/assets/svgs/";

    auto *integrationsBtn = new QPushButton("Integrations");
    integrationsBtn->setStyleSheet(sidebarBtnStyle);
    integrationsBtn->setCheckable(true);
    integrationsBtn->setChecked(true);
    integrationsBtn->setIcon(coloredIcon(iconDir + "integrations.svg", QColor("#aaaaaa")));
    integrationsBtn->setIconSize(QSize(16, 16));

    auto *nativestrapperBtn = new QPushButton("NativeStrapper");
    nativestrapperBtn->setStyleSheet(sidebarBtnStyle);
    nativestrapperBtn->setCheckable(true);
    nativestrapperBtn->setIcon(coloredIcon(iconDir + "bootstrap.svg", QColor("#aaaaaa")));
    nativestrapperBtn->setIconSize(QSize(16, 16));

    auto *deploymentBtn = new QPushButton("Deployment");
    deploymentBtn->setStyleSheet(sidebarBtnStyle);
    deploymentBtn->setCheckable(true);
    deploymentBtn->setIcon(coloredIcon(iconDir + "deployment.svg", QColor("#aaaaaa")));
    deploymentBtn->setIconSize(QSize(16, 16));

    sidebarLayout->addWidget(integrationsBtn);
    sidebarLayout->addWidget(nativestrapperBtn);
    sidebarLayout->addWidget(deploymentBtn);
    sidebarLayout->addStretch();
    mainLayout->addWidget(sidebar);

    auto *stack = new QStackedWidget();
    stack->addWidget(makeIntegrationsPage(script));
    stack->addWidget(makeNativeStrapperPage(script));
    stack->addWidget(makeDeploymentPage(script));
    stack->setCurrentIndex(0);

    mainLayout->addWidget(stack);
    rootLayout->addWidget(mainArea);

    // footer
    auto *footer = new QWidget();
    footer->setObjectName("footer");
    footer->setStyleSheet(
        "QWidget#footer {"
        "  background-color: #1a1a1a;"
        "  border-top: 1px solid #2e2e2e;"
        "}"
    );
    footer->setFixedHeight(48);
    auto *footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(16, 0, 16, 0);
    footerLayout->setSpacing(8);

    QString footerBtnBase =
        "QPushButton {"
        "  border-radius: 2px;"
        "  font-size: 11px;"
        "  padding: 5px 18px;"
        "  font-weight: 250;"
        "}";

    auto *cancelBtn = new QPushButton("Cancel");
    cancelBtn->setFixedHeight(28);
    cancelBtn->setStyleSheet(footerBtnBase +
        "QPushButton {"
        "  background-color: transparent;"
        "  color: #888888;"
        "  border: 1px solid #3a3a3a;"
        "}"
        "QPushButton:hover { background-color: #2a2a2a; color: #cccccc; border-color: #555555; }"
        "QPushButton:pressed { background-color: #222222; }"
    );

    auto *saveBtn = new QPushButton("Save");
    saveBtn->setFixedHeight(28);
    saveBtn->setStyleSheet(footerBtnBase +
        "QPushButton {"
        "  background-color: #1e937c;"
        "  color: #ffffff;"
        "  border: none;"
        "}"
        "QPushButton:hover { background-color: #45d2b5; }"
        "QPushButton:pressed { background-color: #2ac5a6; }"
    );

    footerLayout->addStretch();
    footerLayout->addWidget(cancelBtn);
    footerLayout->addWidget(saveBtn);
    rootLayout->addWidget(footer);

    QObject::connect(cancelBtn, &QPushButton::clicked, [this]() { close(); });
    QObject::connect(saveBtn, &QPushButton::clicked, [this]() {
        ConfigSaving::Save();
        close();
    });

    auto deselectAll = [=]() {
        integrationsBtn->setChecked(false);
        nativestrapperBtn->setChecked(false);
        deploymentBtn->setChecked(false);
    };

    QObject::connect(integrationsBtn, &QPushButton::clicked, [=]() {
        deselectAll(); integrationsBtn->setChecked(true); stack->setCurrentIndex(0);
    });
    QObject::connect(nativestrapperBtn, &QPushButton::clicked, [=]() {
        deselectAll(); nativestrapperBtn->setChecked(true); stack->setCurrentIndex(1);
    });
    QObject::connect(deploymentBtn, &QPushButton::clicked, [=]() {
        deselectAll(); deploymentBtn->setChecked(true); stack->setCurrentIndex(2);
    });

    setCentralWidget(root);
#ifdef _WIN32
    ApplyDarkTitleBar(this);
#endif
}