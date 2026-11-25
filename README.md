# nbody

A real-time N-body gravity simulator with a Vulkan renderer. Simulations are defined in YAML files, and can run on either the GPU (default) or the CPU, with an optional Barnes-Hut approximation for large particle counts.

This is a rewrite of an older project of mine, hopefully a whole lot better this time around.

## Quick start

```bash
git clone https://github.com/Joey574/nbody
cd nbody
cmake -B build
cmake --build build -j
./bin/nbody --cpu --bh -f simulations/big2spiral.yml
```

Press `space` to unpause. The build step assumes the dependencies for your platform are installed, see [Build](#build).

## Build

### Requirements

- CMake, Git, and a C++23 compiler
- OpenMP
- A Vulkan-capable GPU driver (the Vulkan loader is found at runtime)
- Network access on first configure, since CMake fetches its dependencies

### Linux

**1. Install the base dependencies:**

```bash
# Debian / Ubuntu
sudo apt update
sudo apt install build-essential cmake git pkg-config libvulkan1 mesa-vulkan-drivers

# Fedora
sudo dnf install gcc-c++ cmake git pkgconf-pkg-config vulkan-loader mesa-vulkan-drivers

# Arch
sudo pacman -S base-devel cmake git vulkan-icd-loader
```

`mesa-vulkan-drivers` covers AMD and Intel GPUs. For NVIDIA, install the proprietary driver (or `nvidia-utils` on Arch). On Arch, also install the Vulkan driver for your GPU, e.g. `vulkan-radeon` or `vulkan-intel`.

If you use Clang instead of GCC, also install the OpenMP runtime: `libomp-dev` (Debian/Ubuntu), `libomp-devel` (Fedora), or `openmp` (Arch).

**2. Install the windowing dependencies.** Not sure which you use? Run `echo $XDG_SESSION_TYPE`. Installing both sets gives you a single binary that works on either.

*X11:*

```bash
# Debian / Ubuntu
sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev

# Fedora
sudo dnf install libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel libXext-devel

# Arch
sudo pacman -S libx11 libxrandr libxinerama libxcursor libxi libxext
```

*Wayland:*

```bash
# Debian / Ubuntu
sudo apt install libwayland-dev libxkbcommon-dev wayland-protocols extra-cmake-modules

# Fedora
sudo dnf install wayland-devel libxkbcommon-devel wayland-protocols-devel extra-cmake-modules

# Arch
sudo pacman -S wayland libxkbcommon wayland-protocols extra-cmake-modules
```

**3. Clone and build:**

```bash
git clone https://github.com/Joey574/nbody
cd nbody
```

With both sets of windowing dependencies installed (the default):

```bash
cmake -B build
cmake --build build -j
```

With only one set installed, disable the other backend:

```bash
# X11 only
cmake -B build -DGLFW_BUILD_WAYLAND=OFF
cmake --build build -j

# Wayland only
cmake -B build -DGLFW_BUILD_X11=OFF
cmake --build build -j
```

### Windows

Builds use [MSYS2](https://www.msys2.org/) with the UCRT64 toolchain. Open an **MSYS2 UCRT64** shell and run:

```bash
pacman -S --needed \
    git \
    mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-libgomp
```

```bash
git clone https://github.com/Joey574/nbody
cd nbody
cmake -B build
cmake --build build -j
```

The Vulkan runtime ships with your GPU driver, so no extra install is needed. The binary is `bin/nbody.exe`.

### macOS

Install the Xcode Command Line Tools (`xcode-select --install`) and [Homebrew](https://brew.sh/), then:

```bash
brew install cmake libomp molten-vk
```

```bash
git clone https://github.com/Joey574/nbody
cd nbody
cmake -B build \
    -DCMAKE_PREFIX_PATH="$(brew --prefix);$(brew --prefix libomp)"
cmake --build build -j
```

If CMake picks up a Homebrew compiler instead of Apple Clang, force it with `CC=/usr/bin/clang CXX=/usr/bin/clang++` before the configure step.

## Usage

```bash
./bin/nbody [Options] -f path.yml
```

Press `space` to unpause the simulation.

| Option | Description |
| --- | --- |
| `-f`, `--file` | Simulation config file (YAML). **Required.** |
| `--cpu` | Run the computation on the CPU instead of the GPU |
| `--bh` | Use the Barnes-Hut algorithm |
| `-q`, `--quiet` | Quiet performance output |
| `--refresh` | Set the refresh rate of the performance output |

Example scenarios live in `simulations/`. A few to start with:

```bash
# Basic sample
./bin/nbody -f simulations/sample.yml

# My recommendation
./bin/nbody --cpu --bh -f simulations/big2spiral.yml
```

## Tests

```bash
ctest --test-dir build --output-on-failure
```

With multi-config generators (e.g. Visual Studio), add `-C Release`.
