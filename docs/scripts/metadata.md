# The metadata table

Every bootstrap script must define a `metadata` table at the top of the file. This table tells NativeStrapper everything it needs to know about the script before running it.

Example thing:

```lua
metadata = {
    title = "Sober - A Roblox runtime for Linux",
    platform = {"Linux"},
    required = {"strings"},
    uris = {"roblox", "roblox-player"},
    run = "flatpak run org.vinegarhq.Sober %u", -- %u gets replaced with the URI
    capabilities = {"Updates"},
    appdirectories = {
        {path = "/home/%user/.var/app/org.vinegarhq.Sober/data/sober/appData", label = "Sober"}
    }
}
```

---

## Fields

### title
`string` - **Required**

The display name of the script shown in the NativeStrapper UI.

---

### platform
`table of strings` - Optional

The platforms this script supports. If the current platform is not in this list, the script will refuse to install.

Supported values: `"Linux"`, `"Windows"`, `"macOS"`

If omitted, the script is considered cross-platform.

---

### required
`table of strings` - Optional

A list of system commands that must be available for the script to work. NativeStrapper checks for these before installing the script.

```lua
required = {"strings", "flatpak"}
```

---

### uris
`table of strings` - **Required**

The URI schemes this script handles. When a link like `roblox://` is clicked, NativeStrapper uses this to find the right script.

```lua
uris = {"roblox", "roblox-player"}
```

---

### run
`string` - **Required**

The command used to launch Roblox. `%u` is replaced with the URI when launched from a link.

```lua
run = "flatpak run org.vinegarhq.Sober %u"
```

---

### capabilities
`table of strings` - Optional

Tells NativeStrapper which UI features to show for this script.

| Value | Description |
|-------|-------------|
| `"WIP"` | wip lol |

---

### appdirectories
`table of tables` - Optional

A list of app data directories for this script. Used by the settings UI to know where mods and fast flags should be applied.

Each entry has:
- `path` : `string` - The path to the directory
- `label` : `string` - A display name shown in the UI

```lua
appdirectories = {
    {path = "/home/%user/.var/app/org.vinegarhq.Sober/data/sober/appData", label = "Sober"},
}
```

---

## The bootstrap function

Every script must also define a `bootstrap(uri)` function. This is called by NativeStrapper when a Roblox URI is opened.

```lua
function bootstrap(uri)
    NativeStrapper.setStatus("Checking for updates...")
    -- your logic here (checking for updates, updating, etc)
    NativeStrapper.setStatus("Starting Roblox...") -- Launching roblox is handled by the program itself so dw about it
end
```

The `uri` parameter is the full URI that was opened, e.g. `roblox://experiences/start?placeId=1818`.