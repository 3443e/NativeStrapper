# NativeStrapper

A cross-platform Roblox bootstrapper that supports custom launch configurations through lua bootstrap scripts.

## What is NativeStrapper?

NativeStrapper is a bootstrapper for Roblox and Roblox-compatible clients. Instead of hardcoding support for specific clients, NativeStrapper uses **bootstrap scripts** basically lua scripts that define how to launch, update, and configure the client.

This means NativeStrapper can work with:
- **Official Roblox** - On supported platforms
- **Sober** - A native Roblox runtime for Linux
- **Wine-based clients** - Run Windows Roblox clients on Linux
- **Revival clients** - Custom Roblox clients like Pekora/Korone
- **And everything else**

## Features

- Modular, YOU, the user controls how everything works with bootstrap scripts.
- URI registration for all platforms.
- Cross-platform.
- Per-script app data directory management (basically support for multiple appdata folders, useful for roblox revivals with multiple versions and stuff)

## Installation

Download the latest release from [GitHub](https://github.com/3443e/NativeStrapper/releases) and run it. NativeStrapper will guide you through the installation process.

On Linux, NativeStrapper installs to `~/.local/bin/nativestrapper` by default.

## Getting Started

Once installed, open NativeStrapper and click **Import Script** to add a bootstrap script for your Roblox client.

Don't have a script? Check out the community scripts on our [discord server](https://discord.gg/eUSGaxZSBX) or write your own using the [scripting guide](scripts/index.md).