# Building for linux (x86_64)

### Dependencies

Ubuntu/Debian:

```bash
sudo apt update
sudo apt install -y \
    qtbase5-dev \
    qtbase5-dev-tools \
    cmake \
    build-essential \
    libqt5network5 \
    libqt5widgets5
```

### Build

```bash
git clone https://github.com/3443e/NativeStrapper.git
cd NativeStrapper

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The resulting executable will be located at:

```text
build/nativestrapper
```

### Packaging

To create a portable release package:

```bash
mkdir -p release

cp build/nativestrapper release/
cp -r assets release/

cd release
tar -czf ../NativeStrapper-linux.tar.gz .
```