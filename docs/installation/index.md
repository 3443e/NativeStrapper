# Installation

Prebuilt binaries are available for all supported platforms and are recommended for most users.

If you want to build from source instead, see the [Building Guide](../building).

::: warning
(For portable versions) NativeStrapper depends on its internal folder structure, Moving or deleting files may cause it to stop working correctly.
:::

---

## Linux (x86_64) (Portable)

Only a portable version is provided for Linux. You can place and run it anywhere you prefer.

### Steps

* Download the latest Linux release from GitHub
* Extract the archive
* Run the executable:

```bash
./nativestrapper
```

---

## Windows (x64) (Installer)

This is the recommended installation method on Windows.

### Steps

* Download the latest Windows installer from GitHub Releases
* Run the installer
* Follow the setup wizard

Once installed, NativeStrapper can be launched from the Start Menu.

---

## Windows (x64) (Portable)

A portable version is also available for users who prefer a no-install setup.

### Steps

* Download the portable archive from GitHub Releases
* Extract it to a folder of your choice
* Run:

```text
nativestrapper.exe
```

---

# Uninstallation

## Portable version

To uninstall the portable version and restore default roblox stuff (URI handlers, etc.), run:

```bash
nativestrapper --uninstall
```

## Installer version (Windows)

If you installed NativeStrapper using the Windows installer, you can uninstall it normally via:

* Settings -> Apps -> NativeStrapper -> Uninstall
  or
* Control Panel -> Programs and Features
