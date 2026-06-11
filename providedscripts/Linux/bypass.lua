-- sets an fflag or whatever, returns true on success, false on failure
function setfflag(fflag)

end

-- returns the current value of some fflag, can be a string, int, bool etc..
function getfflag(fflag)

end

-- returns a json or something of all fflags
function getfflags()

end

-- something, returns true on success, false on failure
function patchasset(oldassetpath, newassetpath)

end

metadata = {
    title = "NativeStrapper - Bypass update",
    uris = {"roblox", "roblox-player"},
    run = "flatpak run org.vinegarhq.Sober %u",
    platform = {"Linux"},
    appdirectories = {
        {path = "/home/" ..NativeStrapper.Constants.USER.. "/.var/app/org.vinegarhq.Sober/data/sober/appData", label = "Main"}
    },
    capabilities = {"ENABLE_NS_ALL_CAPS"},
    logfolders = {
        "/home/" ..NativeStrapper.Constants.USER.. "/ov2",
        "/home/" ..NativeStrapper.Constants.USER.. "/.var/app/org.vinegarhq.Sober/data/sober/appData/logs"
    },
    nsoverrides = {
        ["SETFFLAG"] = setfflag,
        ["GETFFLAG"] = getfflag,
        ["GETFFLAGS"] = getfflags,
        ["PATCH_ASSET"] = patchasset
    }
}

function bootstrap(uri)
    
    print("uh yes")
    NativeStrapper.log("Up to date!")
    NativeStrapper.setStatus("Launching roblox")
end
