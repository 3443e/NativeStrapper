#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "UserInterface/SettingsWindow.hpp"

SettingsWindow::SettingsWindow() {
    setWindowTitle("NativeStrapper Settings");

    auto *tabs = new QTabWidget(this);

    QWidget *generalTab = new QWidget();
    QVBoxLayout *generalLayout = new QVBoxLayout(generalTab);
    generalLayout->addWidget(new QLabel("tab 1"));
    generalTab->setLayout(generalLayout);

    QWidget *advancedTab = new QWidget();
    QVBoxLayout *advancedLayout = new QVBoxLayout(advancedTab);
    advancedLayout->addWidget(new QLabel("tab 2"));
    advancedTab->setLayout(advancedLayout);

    tabs->addTab(generalTab, "tab 1");
    tabs->addTab(advancedTab, "tab 2");

    auto *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(tabs);

    QWidget *central = new QWidget();
    central->setLayout(mainLayout);
    setCentralWidget(central);
}