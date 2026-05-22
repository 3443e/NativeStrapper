scatter = {
    title = "Sober - A roblox runtime for linux.",
    platform = {"Linux"},
    required = {"strings"},
    uris = {"roblox", "roblox-player"},
    run = "flatpak run org.vinegarhq.Sober %u",
    appdirectories = {
        {path = "/home/%user/.var/app/org.vinegarhq.Sober/data/sober/appData", label = "Sober"}
    }
}

function bootstrap(uri)
    NativeStrapper.setStatus("Checking for updates...")
    local manifestres = NativeStrapper.request({
        url = "https://sober.vinegarhq.org/app",
    })

    local soberlatest
    for match in manifestres.body:gmatch("s%d+%-%d+%-x86_64%-gnu") do
        soberlatest = match
    end

    NativeStrapper.log("Latest Sober: " .. soberlatest)

    local currentver = NativeStrapper.run("strings /home/%user/.var/app/org.vinegarhq.Sober/data/sober/packages/x86_64/com.roblox.client/base.apk | grep -Eo '[0-9]{1,3}\\.[0-9]{1,4}\\.[0-9]{1,4}' | awk -F. '$2 > 700' | tail -1")
    NativeStrapper.log("Current version: " .. currentver)

    -- get latest roblox version
    local apkpureres = NativeStrapper.request({
        url = "https://apkpure.com/roblox-android-2025/com.roblox.client",
    })

    local latestver = apkpureres.body:match("%d+%.%d+%.%d+")
    NativeStrapper.log("Latest version: " .. latestver)

    if currentver == latestver then
        NativeStrapper.log("Already up to date!")
    else
        NativeStrapper.log("Update available: " .. currentver .. " -> " .. latestver)
        NativeStrapper.setStatus("Fetching download URLs...")

        -- get download urls
        local soberupdateapires = NativeStrapper.request({
            url = "https://sober.vinegarhq.org/update",
            headers = {
                ["sober_version"] = soberlatest
            }
        })
        local baseapkurl = -- we do some string::match thing here
        local splitapkurl = -- we do some string::match thing here
        NativeStrapper.log("Downloading base.apk...")
        NativeStrapper.setStatus("Downloading update...")
        NativeStrapper.download(baseapkurl, "/home/%user/.var/app/org.vinegarhq.Sober/data/sober/packages/x86_64/com.roblox.client/base.apk", function(downloaded, total)
            if total > 0 then
                NativeStrapper.setProgress(math.floor(downloaded / total * 100))
            end
        end)

        NativeStrapper.log("Downloading split_config.x86_64.apk...")
        NativeStrapper.download(splitapkurl, "/home/%user/.var/app/org.vinegarhq.Sober/data/sober/packages/x86_64/com.roblox.client/split_config.x86_64.apk", function(downloaded, total)
            if total > 0 then
                NativeStrapper.setProgress(math.floor(downloaded / total * 100))
            end
        end)

        NativeStrapper.log("Update complete!")
    end

    NativeStrapper.setStatus("Starting Roblox...")
end