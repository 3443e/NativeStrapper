metadata = {
    title = "RobloxPlayerBeta.exe - Windows",
    uris = {"roblox", "roblox-player"},
    run = "%LOCALAPPDATA%/idkber %u",
    platform {"Windows"},
    capabilities = {"ENABLE_NS_ALL_CAPS"},
    appdirectories = {
        {path = "C:\\", label = "Main"}
    },
    logfolders = {
        "C:\\",
        "C:\\s"
    }
}

function bootstrap(uri)
    print("uh yes")
    NativeStrapper.log("Up to date!")
    NativeStrapper.setStatus("Launching roblox")
end