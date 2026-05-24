metadata = {
    title = "Sober - A roblox runtime for linux",
    uris = {"roblox", "roblox-player"},
    platform = {"Linux"},
    appdirectories = {
        {path = "/home/a", label = "2021M"},
        {path = "/home/b", label = "2017L"}
    }
}

function bootstrap(uri)
    NativeStrapper.log("Called bootstrap with URI: " .. uri)
    NativeStrapper.setStatus("Checking for updates")

    NativeStrapper.log("Getting current roblox version")
    local home = NativeStrapper.Constants.HOME
    local basepath = home .. "/.var/app/org.vinegarhq.Sober/data/sober/packages/x86_64/com.roblox.client/base.apk"
    local splitpath = home .. "/.var/app/org.vinegarhq.Sober/data/sober/packages/x86_64/com.roblox.client/split_config.x86_64.apk"

    local currentver = nil
    local f = io.open(basepath, "rb")
    if not f then
        NativeStrapper.log("Could not open base.apk")
        currentver = "0.000.0000"
    else
        f:seek("end", -1024 * 1024 * 30) --30 MB
        local data = f:read("*a")
        f:close()

        for major, minor, patch in data:gmatch("(%d%d?)%.(%d%d%d%d?)%.(%d+)") do
            if tonumber(minor) > 700 then
                currentver = major .. "." .. minor .. "." .. patch
            end
        end
    end

    --overwrite the version to 0.000.0000 if there's no split apk
    local f = io.open(splitpath, "rb")
    if not f then
        NativeStrapper.log("Could not open split_config.x86_64.apk")
        currentver = "0.000.0000"
    end

    currentver = (currentver or "0.000.0000")
    print("Current version: " .. currentver)

    NativeStrapper.log("Getting latest roblox version")
    local apkpureres = NativeStrapper.request({url = "https://apkpure.com/roblox-android-2025/com.roblox.client"})
    local latestver = apkpureres.body:match("%d+%.%d+%.%d+")
    latestver = "b"
    print("Latest version: " .. latestver)

    if currentver ~= latestver then
        NativeStrapper.setStatus("Upgrading roblox")
        local manifestres = NativeStrapper.request({url = "https://sober.vinegarhq.org/app"})
        local manifestdata = NativeStrapperJson.decode(manifestres.body)
        local latestsober = nil
        local latestNum = 0
        for key, _ in pairs(manifestdata.v1) do
            local num = tonumber(key:match("s0*(%d+)-"))
            if num and num > latestNum then
                latestNum = num
                latestsober = key
            end
        end

        NativeStrapper.log("Sober latest: " .. latestsober) -- s0106-2350-x86_64-gnu

        local apkpath = "/home/"..NativeStrapper.Constants.USER.."/.var/app/org.vinegarhq.Sober/data/sober/packages/x86_64/com.roblox.client"
        local updateres = NativeStrapper.request({url = "https://sober.vinegarhq.org/update", headers = {["sober_version"] = latestsober}})
        local updatedata = NativeStrapperJson.decode(updateres.body)
        NativeStrapper.download(updatedata.urls["base.apk"], apkpath .. "/base.apk", "base.apk")
        NativeStrapper.download(updatedata.urls["split_config.x86_64.apk"], apkpath .. "/split_config.x86_64.apk", "split_config.x86_64.apk")
    end

    NativeStrapper.log("Up to date!")
    NativeStrapper.setStatus("Launching roblox")
end