# Writing Bootstrap Scripts

Bootstrap scripts are lua files that tell NativeStrapper how to launch, update, and configure a Roblox client.

## How it works

When you launch roblox through nativestrapper, it:

1. Shows the bootstrap window
2. Calls your script's `bootstrap(uri)` function with the full URI
3. Your script does whatever it needs, checks for updates, download files, etc.
4. Roblox launches

## A minimal script

Every bootstrap script needs two things, a `metadata` table and a `bootstrap` function:

```lua
metadata = {
    title = "My epik Script",
    uris = {"roblox", "roblox-player"},
    run = "my-roblox-client.exe %u"
}

function bootstrap(uri)
    NativeStrapper.setStatus("Starting Roblox...")
    -- NativeStrapper will launch using metadata.run automatically
end
```

## File format

Bootstrap scripts are plain `.lua` files. When you import a script in NativeStrapper, it gets copied to:

- **Linux**: `~/.config/nativestrapper/scripts/`
- **Windows**: `%AppData%\nativestrapper\scripts\`

## Next steps

- [metadata table](metadata.md) | all the fields you can set
- [API Reference](api.md) | everything NativeStrapper exposes to your script