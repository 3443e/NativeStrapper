#pragma once
#include <QMainWindow>
#include "BootstrapScripts/ScriptManager.hpp"

class SettingsWindow : public QMainWindow {
    Q_OBJECT
public:
    SettingsWindow(const ScriptManager::BootstrapScript &script);
};