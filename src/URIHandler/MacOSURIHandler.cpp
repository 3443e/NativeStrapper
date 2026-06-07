#include "URIHandler/MainURIHandler.hpp"
#include "AppBundlerGen.hpp"
#include "Logger.hpp"
#include <QCoreApplication>

namespace {
constexpr const char* kBundleIdentifier = "com.scene.nativestrapper.urihandler";
}

bool URIHandler::InstallURIs(const std::string &title, const std::vector<std::string> &uris) {
    LauncherConfig cfg;
    cfg.bundleId = kBundleIdentifier;
    cfg.bootstrapScriptTitle = title;
    cfg.uriSchemes = uris;
    cfg.nativestrapperPaths.insert(cfg.nativestrapperPaths.begin(),
                                   QCoreApplication::applicationFilePath().toStdString());

    std::string label = title.empty() ? std::string("NativeStrapper") : title;
    Logger::Log("Installing macOS URI handler bundle " + cfg.bundleId + " for " + label,
                Logger::LogSeverity::SLOG,
                "URIHandler");
    bool success = runSetup(cfg) == 0;
    Logger::Log(success ? "macOS URI handler registration finished"
                        : "macOS URI handler registration failed",
                success ? Logger::LogSeverity::SSUCCESS : Logger::LogSeverity::SERROR,
                "URIHandler");
    return success;
}

bool URIHandler::UninstallURIs(const std::string &title, const std::vector<std::string> &uris) {
    return true;
}
