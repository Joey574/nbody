# nbody Simulation
A rewrite of an older project of mine, hopefully a whole lot better this time around

## Build
You will need CMake, Ninja, Git, a C++23 compiler, and OpenMP. A Vulkan-capable graphics driver and runtime must also be installed to run the application.
<br>

### Linux
```bash
sudo apt update
sudo apt install 
```

```bash
cmake -B build
cmake --build build -j
```
### Windows

### MacOS
```bash
brew install cmake libomp
```

```bash
cmake -B build -DCMAKE_PREFIX_PATH="$(brew --prefix);$(brew --prefix libomp)"
cmake --build build -j
```

# Usage
To run the program simply run
```bash
./bin/nbody -f simulations/sample.yml

# my recommendation
./bin/nbody --cpu --bh -f simulations/big2spiral.yml
```
