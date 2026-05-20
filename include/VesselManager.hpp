#pragma once

#include <string>
#include <vector>

namespace VesselManager {
    struct AppDataDirectory {
        std::string path;
        std::string label;
    };
    
    struct Vessel {
        bool Active = true;
        std::string RobloxURI;
        std::vector<std::string> RobloxURIs;
        std::string RobloxRunCommand;
        std::string VesselTitle;
        std::vector<AppDataDirectory> AppDataDirectories;
    };
    
    extern std::vector<Vessel> LoadedVessels;

    // this function returns a heap allocated vessel struct, give it a vessel json string.
    Vessel* ParseVessel(std::string);

    enum VesselInstallResult {
        VINSTALLSUCCESS,
        VINSTALLUNKNOWNENVIRONMENT,
    };

    // this function installs the vessel appropriately, it takes in a vessel struct pointer.
    VesselInstallResult InstallVessel(Vessel*);

    enum VesselUninstallResult {
        VUNINSTALLSUCCESS,
        VUNINSTALLFAILED,
        VUNINSTALLUNKNOWNENVIRONMENT
    };

    VesselUninstallResult UninstallVessel(Vessel*);
}
