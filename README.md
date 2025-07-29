# MAKCU C++ Library

High-performance C++ library for MAKCU mouse controllers. Sub-millisecond response times, cross-platform support.

## Prerequisites

- C++17 compiler
- CMake 3.15+
- MAKCU Device (VID:PID = 1A86:55D3)
- Linux: `libudev-dev`, `pkg-config`

## Building

```bash
git clone https://github.com/k4hvh/makcu-cpp
cd makcu-cpp
mkdir build && cd build
cmake ..
make -j$(nproc)  # Linux
# OR cmake --build . --config Release  # Windows
sudo make install  # Install system-wide
```

## Integration

Add to your CMakeLists.txt:
```cmake
find_package(makcu-cpp REQUIRED)
target_link_libraries(your_app PRIVATE makcu::makcu-cpp)
```

Basic usage:
```cpp
#include <makcu.h>

makcu::Device device;
device.connect();
device.mouseMove(100, 0);
device.click(makcu::MouseButton::LEFT);
```

See `examples/` for complete integration examples.

## Examples

```bash
cd examples && ./build.sh  # Build examples
./build/bin/demo            # Run demo
```

## Performance

- Mouse Movement: 0.07ms
- Button Click: 0.16ms
- 28x faster than Python implementation

## Troubleshooting

**Linux permissions:**
```bash
sudo usermod -a -G dialout $USER
```

**Windows:** Check Device Manager for COM port with VID:PID = 1A86:55D3

## License

GNU GPLv3
