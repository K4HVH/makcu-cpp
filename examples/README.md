# MAKCU C++ Library Examples

This directory contains example applications demonstrating how to use the MAKCU C++ library.

## Building Examples

First, install the MAKCU library:

```bash
# Build and install the library
cd ..
mkdir build && cd build
cmake ..
make
sudo make install
```

Then build the examples:

```bash
cd examples
mkdir build && cd build
cmake ..
make
```

## Examples

- `basic_usage.cpp` - Simple example showing basic mouse control operations
- `demo.cpp` - Comprehensive demo with performance tests, gaming scenarios, and all library features

## Running Examples

```bash
# Run basic usage example
./bin/basic_usage

# Run full demo (all library features)
./bin/demo
```

## Using the Library in Your Project

### CMake

```cmake
find_package(makcu-cpp REQUIRED)
target_link_libraries(your_target PRIVATE makcu::makcu-cpp)
```

### Manual Linking

Link against:
- `libmakcu-cpp.so` (Linux) or `makcu-cpp.dll` (Windows)
- System libraries: `pthread`, `setupapi` (Windows), `udev` (Linux)