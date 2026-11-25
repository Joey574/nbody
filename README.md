# nbody
A rewrite of an older project of mine, hopefully a whole lot better this time around

## Build
You will need CMake, Git, a C++23 compiler, and OpenMP. A Vulkan-capable graphics driver and runtime must also be installed to run the application.


### Linux

Install the common dependencies:

```bash
# Debian / Ubuntu
sudo apt update
sudo apt install build-essential cmake git pkg-config libvulkan-dev mesa-vulkan-drivers

# Fedora
sudo dnf install gcc-c++ cmake git pkgconf-pkg-config vulkan-loader-devel mesa-vulkan-drivers

# Arch
sudo pacman -S base-devel cmake git vulkan-icd-loader vulkan-headers
```

If you use Clang instead of GCC, also install the OpenMP runtime (`libomp-dev` on Debian/Ubuntu, `libomp-devel` on Fedora, `openmp` on Arch).

Then install the windowing dependencies for your display server. If you want a single binary that works on both, install both sets and use the default configure step.

#### X11

```bash
# Debian / Ubuntu
sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev

# Fedora
sudo dnf install libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel

# Arch
sudo pacman -S libx11 libxrandr libxinerama libxcursor libxi
```

```bash
cmake -B build -DGLFW_BUILD_WAYLAND=OFF
cmake --build build -j
```

#### Wayland

```bash
# Debian / Ubuntu
sudo apt install libwayland-dev libxkbcommon-dev wayland-protocols extra-cmake-modules

# Fedora
sudo dnf install wayland-devel libxkbcommon-devel wayland-protocols-devel extra-cmake-modules

# Arch
sudo pacman -S wayland libxkbcommon wayland-protocols extra-cmake-modules
```

```bash
cmake -B build -DGLFW_BUILD_X11=OFF
cmake --build build -j
```
### Windows

Builds use [MSYS2](https://www.msys2.org/) with the UCRT64 toolchain. Open an **MSYS2 UCRT64** shell and run:

```bash
pacman -S --needed git \
    mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-libgomp
```

```bash
cmake -B build
cmake --build build -j
```

The Vulkan runtime ships with your GPU driver, so no extra install is needed to run.

### macOS

```bash
brew install cmake libomp molten-vk
```

```bash
cmake -B build \
    -DCMAKE_PREFIX_PATH="$(brew --prefix);$(brew --prefix libomp)"
cmake --build build -j
```

If CMake picks up a Homebrew compiler instead of Apple Clang, force it with `CC=/usr/bin/clang CXX=/usr/bin/clang++` before the configure step.

### Tests

```bash
ctest --test-dir build --output-on-failure
```

## Usage
To run the program simply run the command & press ```space``` to unpause
```bash
./bin/nbody -f simulations/sample.yml

# my recommendation
./bin/nbody --cpu --bh -f simulations/big2spiral.yml
```
