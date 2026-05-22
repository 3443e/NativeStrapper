#include "NetworkManagement.hpp"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>
#include <qdir.h>

// again got this from somewhere idk
std::string NetworkManagement::FetchURL(const std::string &url,  const std::map<std::string, std::string> &headers) {
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QString::fromStdString(url)));
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0");
    for (const auto &[key, val] : headers) {
        request.setRawHeader(QByteArray::fromStdString(key), QByteArray::fromStdString(val));
    }
    
    QNetworkReply *reply = manager.get(request);

    // block until done
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return "";
    }

    std::string result = reply->readAll().toStdString();
    reply->deleteLater();
    return result;
}

std::string NetworkManagement::DownloadFile(const std::string &url, const std::string &out, std::function<void(long long, long long)> progressCallback) {
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QString::fromStdString(url)));
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = manager.get(request);

    QString filePath = QString::fromStdString(out);
    QDir().mkpath(QFileInfo(filePath).absolutePath());

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        reply->deleteLater();
        return "";
    }

    QEventLoop loop;

    QObject::connect(reply, &QNetworkReply::downloadProgress, [&](long long received, long long total) {
        file.write(reply->readAll());
        if (progressCallback) progressCallback(received, total);
    });

    QObject::connect(reply, &QNetworkReply::finished, [&]() {
        file.write(reply->readAll());
        file.close();
        loop.quit();
    });

    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        QFile::remove(filePath);
        return "";
    }

    reply->deleteLater();
    return filePath.toStdString();
}