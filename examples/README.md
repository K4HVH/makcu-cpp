# Integration Examples

Quick integration guide for adding MAKCU to existing projects.

## CMake Integration

**1. Install MAKCU library:**
```bash
cd makcu-cpp && mkdir build && cd build
cmake .. && make && sudo make install
```

**2. Add to your existing CMakeLists.txt:**
```cmake
find_package(makcu-cpp REQUIRED)
target_link_libraries(your_existing_target PRIVATE makcu::makcu-cpp)
```

**3. Use in your code:**
```cpp
#include <makcu.h>

makcu::Device device;
device.connect();
device.mouseMove(100, 0);
```

## Manual Compilation

**Linux:**
```bash
g++ -std=c++17 -I/usr/local/include/makcu your_app.cpp -lmakcu-cpp -lpthread -ludev
```

**Windows:** Add include/lib directories and link `makcu-cpp.lib` + `setupapi.lib`

## Bundle with App

Copy library files to your project and link directly:
```cmake
target_link_libraries(your_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libmakcu-cpp.so)
```

## Common Issues

- **"makcu.h not found"**: Run `sudo make install` 
- **"cannot find -lmakcu-cpp"**: Run `sudo ldconfig`
- **DLL not found (Windows)**: Copy `makcu-cpp.dll` next to exe

## Examples

```bash
./build.sh          # Build examples  
./build/bin/demo    # Run demo
```
