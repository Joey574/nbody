# nbody Simulation
A rewrite of an older project of mine, hopefully a whole lot better this time around

## Build
Dependencies are automatically brought in, except for vulkan, which must be installed ahead of time 
<br>
https://www.vulkan.org/

```
cmake -B build -G Ninja
cmake --build build -j
```

# Usage
To run the program simply run
```
./bin/nbody -f simulations/sample.yml
```
