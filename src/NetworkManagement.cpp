#include "NetworkManagement.hpp"
#include <curl/curl.h>
#include <QDir>
#include <QFileInfo>
#include <fstream>

// help
static size_t WriteStringCallback(void* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

static size_t WriteFileCallback(void* ptr, size_t size, size_t nmemb, std::ofstream* file) {
    file->write((char*)ptr, size * nmemb);
    return size * nmemb;
}

std::string NetworkManagement::FetchURL(const std::string& url, const std::map<std::string, std::string>& headers) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    std::string response;
    struct curl_slist* headerList = nullptr;

    for (const auto& [key, val] : headers) {
        headerList = curl_slist_append(headerList, (key + ": " + val).c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteStringCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headerList);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "";
    }
    return response;
}

std::string NetworkManagement::DownloadFile(const std::string& url, const std::string& out, std::function<void(long long, long long)> progressCallback) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    QDir().mkpath(QFileInfo(QString::fromStdString(out)).absolutePath());
    std::ofstream file(out, std::ios::binary);
    if (!file.is_open()) {
        curl_easy_cleanup(curl);
        return "";
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    file.close();

    if (res != CURLE_OK) {
        std::remove(out.c_str());
        return "";
    }
    return out;
}