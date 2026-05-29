# NativeStrapper API Reference

Bootstrap scripts have access to the `NativeStrapper` global table which provides functions for making HTTP requests, downloading files, running commands, and updating the UI.

---

## NativeStrapper.log(message)

Logs a message to the bootstrap window and the console.

**Parameters:**

- `message` : `string`, The message to log.

**Example:**
```lua
NativeStrapper.log("Checking for updates...")
```

---

## NativeStrapper.setStatus(text)

Updates the status text shown in the bootstrap window.

**Parameters:**

- `text` : `string`, The status text to display on the bootstrap window.

**Example:**
```lua
NativeStrapper.setStatus("Downloading update...")
```

---

## NativeStrapper.run(command)

Runs a shell command and returns its stdout as a string. (please avoid using this)

**Parameters:**

- `command` : `string`, The shell command to run.

**Returns:** `string`, The stdout output of the command, trimmed.

**Example:**
```lua
local version = NativeStrapper.run("cat /etc/os-release | grep VERSION_ID")
NativeStrapper.log("OS version: " .. version)
```

---

## NativeStrapper.request(options)

Makes an HTTP GET request and returns the response as a strinh.

**Parameters:**

- `options` - `table`:
    - `url` - `string`, The URL to request.
    - `headers` - `table` (optional), Key-value pairs of request headers.

**Returns:** `table`:
- `body` : `string`, The response body.
- `status` : `number`, The HTTP status code.

**Example:**
```lua
local res = NativeStrapper.request({
    url = "https://sober.vinegarhq.org/app",
    headers = {
        ["User-Agent"] = "NativeStrapper"
    }
})
NativeStrapper.log("Response: " .. res.body)
```

---

## NativeStrapper.download(url, out, label)

Downloads a file from a URL to a local path, showing progress in the bootstrap window.

**Parameters:**

- `url` : `string`, The URL to download from.
- `out` : `string`, The local path to save the file to.
- `label` : `string` (optional) A label shown in the bootstrap window during download. defaults to `"Downloading..."`.

**Returns:** `boolean` - `true` on success, `false` on failure.

**Example:**
```lua
local ok = NativeStrapper.download(
    "https://example.com/roblox.apk",
    "/tmp/roblox.apk",
    "Downloading Roblox"
)
if not ok then
    NativeStrapper.log("Download failed!")
end
```

---

## NativeStrapper.Constants

A read-only table of useful constants.

| Key | Type | Description |
|-----|------|-------------|
| `USER` | `string` | The current system username. |
| `HOME` | `string` | The current user's home directory. |
| `PLATFORM` | `string` | The current platform (`"Linux"`, `"Windows"`, `"macOS"`). |

**Example:**
```lua
local home = NativeStrapper.Constants.HOME
local path = home .. "/.var/app/org.vinegarhq.Sober"
```

---

## NativeStrapperJson

A JSON parsing library available to all scripts. (https://github.com/rxi/json.lua)

### NativeStrapperJson.decode(str)

Parses a JSON string into a Lua table.

**Example:**
```lua
local data = NativeStrapperJson.decode('{"version": "2.721.1108"}')
print(data.version) -- 2.721.1108
```

### NativeStrapperJson.encode(table)

Converts a Lua table to a JSON string.

**Example:**
```lua
local str = NativeStrapperJson.encode({version = "2.721.1108"})
print(str) -- {"version":"2.721.1108"}
```