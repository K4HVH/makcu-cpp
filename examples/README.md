# MAKCU C++ Library Examples

This directory contains example applications and build scripts showing how to integrate the MAKCU C++ library into your own projects.

## 📁 Files

- `basic_usage.cpp` - Simple example showing basic mouse control operations
- `demo.cpp` - Comprehensive demo with performance tests, gaming scenarios, and all library features
- `CMakeLists.txt` - CMake configuration for building examples
- `build.sh` / `build.bat` - Build scripts showing library integration

## 🚀 Quick Start

**Linux:**

```bash
./build.sh
```

**Windows:**

```cmd
build.bat
```

## 📚 Integration Guide for Your Own Projects

### Method 1: CMake with find_package (Recommended)

This is the cleanest approach for CMake-based projects.

#### Step 1: Install MAKCU library system-wide

```bash
# Linux
cd /path/to/makcu-cpp
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install

# Windows  
cd C:\path\to\makcu-cpp
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
cmake --build . --target install
```

#### Step 2: Create your project

```bash
MyProject/
├── src/
│   └── main.cpp
└── CMakeLists.txt
```

#### Step 3: CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyMakcuApp VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the MAKCU library
find_package(makcu-cpp REQUIRED)

# Create your executable
add_executable(${PROJECT_NAME} src/main.cpp)

# Link against MAKCU library
target_link_libraries(${PROJECT_NAME} PRIVATE makcu::makcu-cpp)
```

#### Step 4: Your application code

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
        
        // Your MAKCU operations here
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

#### Step 5: Build your project

```bash
mkdir build && cd build
cmake ..
make  # Linux
# OR
cmake --build . --config Release  # Windows
```

### Method 2: Manual Compilation

For simple projects or when you don't want to use CMake.

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

### Method 3: Portable Distribution

For distributing your application with the library included.

**Project Structure:**

```bash
MyApp/
├── src/
│   └── main.cpp
├── lib/
│   ├── include/           # Copy from installed headers
│   │   ├── makcu.h
│   │   ├── serialport.h
│   │   └── makcu_export.h
│   └── windows/           # Windows libraries
│       ├── makcu-cpp.lib  # Import library
│       └── makcu-cpp.dll  # Runtime library
│   └── linux/             # Linux libraries
│       ├── libmakcu-cpp.so
│       └── libmakcu-cpp.a
└── CMakeLists.txt
```

**CMakeLists.txt for portable distribution:**

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyMakcuApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers from local lib directory
include_directories(lib/include)

# Create executable
add_executable(${PROJECT_NAME} src/main.cpp)

# Platform-specific linking
if(WIN32)
    target_link_libraries(${PROJECT_NAME} 
        PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/makcu-cpp.lib
        setupapi
    )
    # Copy DLL to output directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/makcu-cpp.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
else()
    target_link_libraries(${PROJECT_NAME} 
        PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/libmakcu-cpp.so
        pthread
        udev
    )
endif()
```

## 🔧 Troubleshooting Integration

### Common Issues

#### 1. "makcu.h not found"

- Ensure library is installed: `sudo make install` (Linux) or `cmake --build . --target install` (Windows)
- Check include paths in your build system

#### 2. "cannot find -lmakcu-cpp" (Linux)

- Library not installed system-wide
- Try: `sudo ldconfig` after installation
- Check `/usr/local/lib` contains `libmakcu-cpp.so`

#### 3. "unresolved external symbol" (Windows)

- Missing `makcu-cpp.lib` import library
- Ensure you're linking against the `.lib` file, not the `.dll`

#### 4. "DLL not found" at runtime (Windows)

- `makcu-cpp.dll` not in PATH or application directory
- Copy DLL next to your executable

#### 5. CMake can't find the package**

- Library not installed in standard CMake paths
- Try: `cmake -DCMAKE_PREFIX_PATH=/path/to/installation ..`

### Verification Steps

**1. Check installation:**

```bash
# Linux
ls -la /usr/local/lib/libmakcu-cpp*
ls -la /usr/local/include/makcu/

# Windows
dir "C:\Program Files\makcu-cpp\lib\"
dir "C:\Program Files\makcu-cpp\include\makcu\"
```

**2. Test minimal example:**
Use the `basic_usage.cpp` example as a starting point for your integration.

**3. Check library dependencies:**

```bash
# Linux - check what libraries your executable needs
ldd your_app

# Windows - use Dependency Walker or similar tools
```

## 📋 Examples Output

When you run the build scripts, you'll see executables created in `build/bin/`:

**Running the examples:**

```bash
# Linux
./build/bin/basic_usage
./build/bin/demo

# Windows  
build\bin\Release\basic_usage.exe
build\bin\Release\demo.exe
```

The examples demonstrate:

- **basic_usage**: Device connection, simple mouse operations
- **demo**: Performance testing, gaming scenarios, advanced features

Use these as templates for your own MAKCU-enabled applications!
