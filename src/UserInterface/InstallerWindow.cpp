#include "UserInterface/InstallerWindow.hpp"
#include "NativeStrapper/Installer.hpp"
#include "ConfigSaving.hpp"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QCoreApplication>
#include <QProcess>
#include <QApplication>

InstallerWindow::InstallerWindow() {
    setWindowTitle("NativeStrapper");
    setFixedSize(400, 240);

    auto *root = new QWidget();
    auto *layout = new QVBoxLayout(root);
    layout->setContentsMargins(0, 20, 0, 20);
    layout->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    layout->setSpacing(6);
    layout->addStretch(5);

    QLabel *imageLabel = new QLabel();
    QPixmap pixmap(QCoreApplication::applicationDirPath() + "/assets/NativeStrapper.png");
    imageLabel->setPixmap(pixmap.scaled(160, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(imageLabel, 0, Qt::AlignHCenter);

    layout->addSpacing(8);

    QLabel *titleLabel = new QLabel("Welcome to NativeStrapper!");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 16px; color: #cccccc;");
    layout->addWidget(titleLabel, 0, Qt::AlignHCenter);

    QLabel *subtitleLabel = new QLabel("Choose where to install NativeStrapper.");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("font-size: 10px; color: #888888;");
    layout->addWidget(subtitleLabel, 0, Qt::AlignHCenter);

    layout->addSpacing(8);

    // path row
    QWidget *pathRow = new QWidget();
    QHBoxLayout *pathLayout = new QHBoxLayout(pathRow);
    pathLayout->setContentsMargins(0, 0, 0, 0);
    pathLayout->setSpacing(4);

    pathEdit = new QLineEdit();
    pathEdit->setText(Installer::GetDefaultInstallLocation());
    pathEdit->setFixedWidth(280);
    pathEdit->setStyleSheet(
        "QLineEdit {"
        "  background-color: #2e2e2e;"
        "  color: #cccccc;"
        "  border: 1px solid #555555;"
        "  border-radius: 3px;"
        "  padding: 3px 6px;"
        "  font-size: 10px;"
        "}"
    );
    pathLayout->addWidget(pathEdit);

    QPushButton *browseBtn = new QPushButton("Browse");
    browseBtn->setFixedWidth(60);
    QObject::connect(browseBtn, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getSaveFileName(
            this, "Install Location", pathEdit->text()
        );
        if (!path.isEmpty()) pathEdit->setText(path);
    });
    pathLayout->addWidget(browseBtn);
    layout->addWidget(pathRow, 0, Qt::AlignHCenter);

    layout->addSpacing(8);

    installBtn = new QPushButton("Install");
    installBtn->setFixedWidth(100);
    QObject::connect(installBtn, &QPushButton::clicked, [this]() {
        doInstall();
    });
    layout->addWidget(installBtn, 0, Qt::AlignHCenter);

    layout->addStretch();
    setCentralWidget(root);
}

void InstallerWindow::doInstall() {
    std::string path = pathEdit->text().toStdString();

    installBtn->setEnabled(false);
    installBtn->setText("Installing...");

    if (!Installer::Install(path)) {
        QMessageBox::critical(this, "Error", "Failed to install NativeStrapper.");
        installBtn->setEnabled(true);
        installBtn->setText("Install");
        return;
    }

    // save installation path thing
    ConfigSaving::Save();

    QMessageBox::information(this, "Installed!", "NativeStrapper has been installed successfully.\n\nIt will now relaunch from the installed location.");

    // relaunch from install location
    QProcess::startDetached(QString::fromStdString(path), QApplication::arguments());
    QApplication::quit();
}