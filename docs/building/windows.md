# Building for windows (x64)

### Requirements

* MinGW-w64
* CMake
* Qt 5.15.2 (MinGW 8.1 64-bit)
* NSIS (optional, for building the installer)

### Build

```bash
git clone https://github.com/3443e/NativeStrapper.git
cd NativeStrapper

cmake -B build ^
    -G "MinGW Makefiles" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++"

cmake --build build
```

The resulting executable will be located at:

```text
build\nativestrapper.exe
```

### Portable Package

Create a `release` directory and copy:

```text
build\nativestrapper.exe
assets\
```

Additionally copy the following Qt libraries:

```text
Qt5Core.dll
Qt5Gui.dll
Qt5Widgets.dll
Qt5Network.dll
```

And the MinGW runtime libraries:

```text
libgcc_s_seh-1.dll
libstdc++-6.dll
libwinpthread-1.dll
```

Also create:

```text
release\platforms\
release\styles\
```

and copy:

```text
platforms\qwindows.dll
styles\qwindowsvistastyle.dll
```

The resulting `release` directory can be distributed as a portable version of NativeStrapper.

### Installer

To build the installer:

```cmd
makensis installer.nsi
```

The generated installer will be:

```text
NativeStrapper-Setup.exe
```
