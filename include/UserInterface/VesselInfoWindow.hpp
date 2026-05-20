#pragma once
#include "VesselManager.hpp"
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>

class VesselInfoWindow : public QDialog {
public:
    VesselInfoWindow(const VesselManager::Vessel &vessel, QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("Vessel Info");
        setFixedSize(300, 120);
        auto *layout = new QVBoxLayout(this);

        auto makeRow = [&](const QString &key, const std::string &val) {
            auto *lbl = new QLabel(QString("<b>%1:</b> %2").arg(key, QString::fromStdString(val)));
            lbl->setStyleSheet("font-size: 11px; color: #cccccc;");
            lbl->setWordWrap(true);
            layout->addWidget(lbl);
        };

        makeRow("URI", vessel.RobloxURI);
        makeRow("Run Command", vessel.RobloxRunCommand);
        //makeRow("AppData Dir", vessel.RobloxAppDataDirectory);

        setStyleSheet("background-color: #2a2a2a;");
    }
};