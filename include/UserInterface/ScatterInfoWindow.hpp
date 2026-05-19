#pragma once
#include "ScatterManager.hpp"
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>

class ScatterInfoWindow : public QDialog {
public:
    ScatterInfoWindow(const ScatterManager::Scatter &scatter, QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("Scatter Info");
        setFixedSize(300, 120);
        auto *layout = new QVBoxLayout(this);

        auto makeRow = [&](const QString &key, const std::string &val) {
            auto *lbl = new QLabel(QString("<b>%1:</b> %2").arg(key, QString::fromStdString(val)));
            lbl->setStyleSheet("font-size: 11px; color: #cccccc;");
            lbl->setWordWrap(true);
            layout->addWidget(lbl);
        };

        makeRow("URI", scatter.RobloxURI);
        makeRow("Run Command", scatter.RobloxRunCommand);
        makeRow("AppData Dir", scatter.RobloxAppDataDirectory);

        setStyleSheet("background-color: #2a2a2a;");
    }
};