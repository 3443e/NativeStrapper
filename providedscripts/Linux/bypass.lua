metadata = {
    title = "NativeStrapper - Bypass update",
    uris = {"roblox", "roblox-player"},
    run = "flatpak run org.vinegarhq.Sober %u",
    platform = {"Linux"},
    appdirectories = {
        {path = "/home/" ..NativeStrapper.Constants.USER.. "/.var/app/org.vinegarhq.Sober/data/sober/appData", label = "Main"}
    },
    log_folders = {"/home/" ..NativeStrapper.Constants.USER.. "/.var/app/org.vinegarhq.Sober/data/sober/appData/logs"}
}

function bootstrap(uri)
    print("uh yes")
    NativeStrapper.log("Up to date!")
    NativeStrapper.setStatus("Launching roblox")
end