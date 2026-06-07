#pragma once
#include <string>
#include <vector>

namespace URIHandler {
    bool InstallURIs(const std::string &title, const std::vector<std::string> &uris);
    bool UninstallURIs(const std::string &title, const std::vector<std::string> &uris);
}