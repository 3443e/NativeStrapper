#pragma once
#include <string>
#include <functional>

namespace NetworkManagement {
    // simple fetch, returns response body
    std::string FetchURL(const std::string &url);

    // download a file to a directory, calls progressCallback(downloaded, total) as it goes
    // returns path to downloaded file, empty string on failure
    std::string DownloadFile(const std::string &url, const std::string &out, std::function<void(long long, long long)> progressCallback = nullptr);
}