# MAKCU C++ Library

High-performance C++ library for MAKCU mouse controllers with cross-platform support and sub-millisecond response times.

## 🚀 Performance Highlights

- **Sub-0.1ms Command Execution**: Measured 0.06-0.2ms for core operations
- **Cross-Platform**: Windows (DLL) and Linux (shared/static library) support
- **Zero-Delay Architecture**: Optimized serial communication with minimal timeouts
- **28x Faster** than Python implementation
- **Gaming-Ready**: Ideal for 360Hz+ competitive gaming
- **Library Architecture**: Reusable shared/static library with external API

## 📦 Library Structure

This project builds as a **shared/static library** that can be used by external applications:

```
makcu-cpp/
├── makcu-cpp/
│   ├── include/          # Public API headers
│   │   ├── makcu.h       # Main device API  
│   │   ├── serialport.h  # Serial communication API
│   │   └── makcu_export.h # Export macros for shared library
│   └── src/              # Library implementation
├── examples/             # Example applications
│   ├── basic_usage.cpp   # Simple usage example
│   ├── demo.cpp          # Comprehensive demo
│   └── CMakeLists.txt    # Builds examples against installed library
└── CMakeLists.txt        # Library build configuration
```

## 🛠️ Prerequisites

### Windows
- **Visual Studio 2019+** with C++17 support
- **Windows 10/11**
- **MAKCU Device** (VID:PID = 1A86:55D3)

### Linux
- **GCC 7+ or Clang 6+** with C++17 support
- **CMake 3.15+**
- **libudev development files** (`libudev-dev` on Ubuntu/Debian)
- **pkg-config**
- **MAKCU Device** (VID:PID = 1A86:55D3)

## 🔧 Building the Library

### Linux

**Install Dependencies:**
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential cmake pkg-config libudev-dev

# CentOS/RHEL/Fedora
sudo dnf install gcc-c++ cmake pkg-config systemd-devel

# Arch Linux
sudo pacman -S base-devel cmake pkg-config systemd
```

**Build Library:**
```bash
git clone https://github.com/your-repo/makcu-cpp
cd makcu-cpp

# Build both shared and static libraries (default)
mkdir build && cd build
cmake ..
make -j$(nproc)

# Install both libraries system-wide
sudo make install
```

**Using Build Script:**
```bash
./build.sh  # Automated build with dependency checking
```

### Windows

**Build with Visual Studio:**
```cmd
git clone https://github.com/your-repo/makcu-cpp
cd makcu-cpp
mkdir build && cd build

# Build both shared (.dll) and static (.lib) libraries
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release

# Install both libraries
cmake --build . --target install
```

**Using Build Script:**
```cmd
build.bat  # Automated build with Visual Studio detection
```

## 📚 Using the Library in Your Project

### CMake Integration (Recommended)

After installing the library, create your project with this structure:

```
MyProject/
├── src/
│   └── main.cpp
└── CMakeLists.txt
```

**CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.15)
project(MyMakcuApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the MAKCU library
find_package(makcu-cpp REQUIRED)

# Create your executable
add_executable(${PROJECT_NAME} src/main.cpp)

# Link against MAKCU library
target_link_libraries(${PROJECT_NAME} PRIVATE makcu::makcu-cpp)
```

**Your application code:**
```cpp
// src/main.cpp
#include <makcu.h>
#include <iostream>

int main() {
    try {
        makcu::Device device;
        if (!device.connect()) {
            std::cout << "Failed to connect to MAKCU device\n";
            return 1;
        }
        
        std::cout << "Connected! Firmware: " << device.getVersion() << "\n";
        
        // Your MAKCU operations
        device.mouseMove(100, 0);
        device.click(makcu::MouseButton::LEFT);
        
        device.disconnect();
    }
    catch (const makcu::MakcuException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

**Build your project:**
```bash
mkdir build && cd build
cmake ..
make  # Linux
# OR
cmake --build . --config Release  # Windows
```

### Manual Compilation

For simple projects without CMake:

**Linux:**
```bash
g++ -std=c++17 \
    -I/usr/local/include/makcu \
    your_app.cpp \
    -lmakcu-cpp \
    -lpthread \
    -ludev \
    -o your_app
```

**Windows (Visual Studio IDE):**

1. **Create/Open your Visual Studio project**
2. **Right-click your project** → Properties
3. **Configuration Properties** → C/C++ → **General**
   - Additional Include Directories: `C:\Program Files\makcu-cpp\include\makcu`
4. **Configuration Properties** → Linker → **General**
   - Additional Library Directories: `C:\Program Files\makcu-cpp\lib`
5. **Configuration Properties** → Linker → **Input**
   - Additional Dependencies: Add `makcu-cpp.lib` and `setupapi.lib`
6. **Copy `makcu-cpp.dll`** next to your executable or ensure it's in PATH

### Distribution Options

**Option 1: System Installation** (Recommended)
- Install library system-wide (`make install`)
- Users link against installed library
- Smallest application size

**Option 2: Portable Distribution**
- Bundle library files with your application
- Include headers, .lib/.dll (Windows) or .so/.a (Linux)
- Self-contained but larger distribution

See `examples/README.md` for detailed integration guides and troubleshooting.

## 🚀 Quick Start

### Basic Usage

```cpp
#include <makcu.h>
#include <iostream>

int main() {
    try {
        // Find available devices
        auto devices = makcu::Device::findDevices();
        if (devices.empty()) {
            std::cout << "No MAKCU devices found.\n";
            return 1;
        }

        // Connect to first device
        makcu::Device device;
        if (!device.connect(devices[0].port)) {
            std::cout << "Failed to connect to device.\n";
            return 1;
        }

        std::cout << "Connected! Firmware: " << device.getVersion() << "\n";

        // Basic operations
        device.mouseMove(100, 0);                    // Move right
        device.click(makcu::MouseButton::LEFT);      // Click
        device.mouseWheel(3);                        // Scroll up

        device.disconnect();
        std::cout << "Demo completed!\n";
    }
    catch (const makcu::MakcuException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### High-Performance Gaming Setup

```cpp
#include <makcu.h>

int main() {
    makcu::Device device;
    
    // Auto-detect and connect
    if (!device.connect()) {
        return 1;
    }
    
    // Enable gaming mode for maximum performance
    device.enableHighPerformanceMode(true);
    
    // Ultra-fast operations (sub-millisecond)
    device.click(makcu::MouseButton::LEFT);     // ~0.16ms
    device.mouseMove(0, -2);                    // ~0.07ms (recoil control)
    device.mouseWheel(1);                       // ~0.05ms (weapon switch)
    
    return 0;
}
```

### Batch Operations

```cpp
// Execute multiple commands efficiently
auto batch = device.createBatch();
batch.move(50, 0)
     .click(makcu::MouseButton::LEFT)
     .moveSmooth(0, 50, 10)
     .scroll(3);
batch.execute(); // All commands sent together
```

## 🎮 Example Applications

After installing the library, build and run the examples:

```bash
# Build examples
cd examples
mkdir build && cd build
cmake ..
make

# Run basic example
./bin/basic_usage

# Run comprehensive demo (performance tests, gaming scenarios)
./bin/demo
```

The examples demonstrate:
- **basic_usage.cpp**: Simple library usage
- **demo.cpp**: Performance testing, gaming scenarios, drag operations, input locking, async operations

## 📊 Performance Results

### Measured Performance (Linux/Windows)

| Operation | Average Latency | Gaming Suitability |
|-----------|----------------|-------------------|
| Mouse Movement | 0.07ms | Perfect (360Hz+) |
| Button Click | 0.16ms | Perfect (360Hz+) |
| Wheel Scroll | 0.05ms | Perfect (360Hz+) |
| Batch Commands (9 ops) | <0.1ms | Perfect (any Hz) |
| State Query (cached) | 0.001ms | Instant |

### vs Python Implementation

| Operation | makcu-py-lib v2.0 | C++ Library | Improvement |
|-----------|-------------------|-------------|-------------|
| Mouse Movement | 2ms | **0.07ms** | **28x faster** |
| Button Click | 1-2ms | **0.16ms** | **6-12x faster** |
| Batch Commands | 3ms | **<0.1ms** | **30x+ faster** |

## 🔒 Advanced Features

### Input Locking
```cpp
// Block physical input while maintaining software control
device.lockMouseX(true);        // Block horizontal movement
device.lockMouseLeft(true);     // Block left clicks

// Software control still works
device.mouseMove(100, 50);      // Y-axis not locked
device.click(makcu::MouseButton::RIGHT); // Right button not locked

// Check states instantly (cached)
if (device.isMouseLeftLocked()) {
    std::cout << "Left button is locked\n";
}
```

### Event Monitoring
```cpp
// Real-time button monitoring
device.setMouseButtonCallback([](makcu::MouseButton button, bool pressed) {
    if (pressed && button == makcu::MouseButton::LEFT) {
        std::cout << "Left click detected!\n";
    }
});

device.enableButtonMonitoring(true);
```

### Performance Profiling
```cpp
// Enable profiling
makcu::PerformanceProfiler::enableProfiling(true);

// Get timing statistics
auto stats = makcu::PerformanceProfiler::getStats();
for (const auto& [command, data] : stats) {
    auto [count, total_us] = data;
    double avg_us = static_cast<double>(total_us) / count;
    std::cout << command << ": avg " << avg_us << "μs\n";
}
```

## 🛡️ Error Handling

```cpp
try {
    makcu::Device device;
    device.connect();
    device.click(makcu::MouseButton::LEFT);
} catch (const makcu::ConnectionException& e) {
    std::cerr << "Connection error: " << e.what() << std::endl;
} catch (const makcu::CommandException& e) {
    std::cerr << "Command error: " << e.what() << std::endl;
} catch (const makcu::TimeoutException& e) {
    std::cerr << "Timeout error: " << e.what() << std::endl;
}
```

## 🔧 Build Options

| Option | Description | Default |
|--------|-------------|---------|
| `BUILD_SHARED_ONLY` | Build only shared library (.so/.dll) | OFF |
| `BUILD_STATIC_ONLY` | Build only static library (.a/.lib) | OFF |

By default, both shared and static libraries are built. Use these options to build only one type:

```bash
# Build only shared library
cmake -DBUILD_SHARED_ONLY=ON ..

# Build only static library  
cmake -DBUILD_STATIC_ONLY=ON ..

# Build both (default)
cmake ..
```

## 🔧 Troubleshooting

### Linux Device Permissions
```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER
# Log out and back in

# Check device detection
lsusb | grep 1a86:55d3
ls -la /dev/ttyUSB* /dev/ttyACM*
```

### Windows Connection Issues
- Check Device Manager for COM port
- Verify VID:PID = 1A86:55D3
- Try different USB ports

### Performance Optimization
1. **Enable High-Performance Mode**
   ```cpp
   device.enableHighPerformanceMode(true);
   ```

2. **Use Batch Commands**
   ```cpp
   auto batch = device.createBatch();
   // Add multiple commands
   batch.execute();
   ```

3. **Cache State Queries**
   ```cpp
   // Fast (cached)
   bool locked = device.isMouseLeftLocked();
   ```

## 📝 API Reference

### Core Classes
- `makcu::Device` - Main device control class
- `makcu::Device::BatchCommandBuilder` - Batch command execution
- `makcu::PerformanceProfiler` - Performance monitoring

### Mouse Operations
- `click(MouseButton)` - Single click
- `mouseMove(x, y)` - Instant movement  
- `mouseMoveSmooth(x, y, segments)` - Smooth interpolated movement
- `mouseWheel(delta)` - Scroll wheel

### State Management
- `lockMouseX/Y/Left/Right(bool)` - Input locking
- `isMouseXLocked()` etc. - Cached state queries
- `mouseButtonState(button)` - Real-time button state

See the `examples/demo.cpp` for comprehensive API usage.

## 📝 License

GNU GPLv3 License

## ⭐ Acknowledgements

- [Makcu Discord Server](https://discord.gg/frvh3P4Qeg) community
- [Makcu Python Library](https://github.com/SleepyTotem/makcu-py-lib) by [SleepyTotem](https://github.com/SleepyTotem)

---

## 🚀 Why Use This Library?

✅ **Sub-millisecond performance** - Perfect for competitive gaming  
✅ **Cross-platform** - Same API on Windows and Linux  
✅ **Production-ready** - Comprehensive error handling and testing  
✅ **Easy integration** - Standard CMake `find_package()` support  
✅ **Flexible** - Both shared and static library options  
✅ **Well-documented** - Complete examples and API reference