# MAKCU C++ Library - Quick Start Guide

This guide will help you get up and running with the MAKCU C++ Library in just a few minutes.

## Prerequisites

1. **Hardware**: MAKCU device with CH343 chipset
2. **Software**: 
   - Windows 10/11
   - Visual Studio 2019 or later (with C++ desktop development workload)
   - OR CMake 3.16+ and any C++17 compiler

## Step 1: Build the Library

### Option A: Using Visual Studio (Recommended)

1. Open `makcu-cpp.sln` in Visual Studio
2. Select **Release** configuration and **x64** platform
3. Press **F7** or go to **Build → Build Solution**
4. The executable will be in `x64/Release/makcu-cpp.exe`

### Option B: Using CMake

1. Open Command Prompt or PowerShell in the project directory
2. Run the build script:
   ```batch
   build.bat
   ```
   OR manually:
   ```batch
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build . --config Release
   ```

## Step 2: Connect Your Device

1. Connect your MAKCU device to a USB port
2. Windows should automatically install drivers
3. Check Device Manager to see the COM port (e.g., COM3)

## Step 3: Run Your First Example

### Basic Demo
```batch
# From the build directory (Visual Studio)
x64\Release\makcu-cpp.exe

# OR from CMake build directory
Release\makcu-demo.exe
```

### Mouse Monitoring
```batch
Release\mouse-monitor.exe
```

### Keyboard Automation
```batch
Release\keyboard-automation.exe
```

## Step 4: Your First Program

Create a new file `my_first_makcu.cpp`:

```cpp
#include "include/makcu.h"
#include <iostream>

int main() {
    try {
        // Create device and connect
        makcu::Device device;
        if (!device.connect()) {
            std::cout << "No device found!" << std::endl;
            return 1;
        }
        
        std::cout << "Connected! Testing basic functions..." << std::endl;
        
        // Type some text
        device.typeString("Hello from MAKCU!");
        
        // Press Enter
        device.keyPress(makcu::KeyCode::KEY_ENTER);
        
        // Move mouse cursor
        device.mouseMove(100, 50);
        
        // Click
        device.mouseClick(makcu::MouseButton::LEFT);
        
        std::cout << "Demo completed!" << std::endl;
        
    } catch (const makcu::MakcuException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

Compile and link against the library:
```batch
# If using Visual Studio, add to your project and link against the built library
# If using CMake, add to CMakeLists.txt as an executable
```

## Common Issues & Solutions

### "Device not found"
- Check USB connection
- Verify device appears in Device Manager
- Try different USB port
- Ensure no other software is using the device

### "Access denied" or connection errors
- Run as Administrator
- Check if antivirus is blocking
- Ensure proper drivers are installed

### Compilation errors
- Verify C++17 support is enabled
- Check all include paths are correct
- Ensure setupapi.lib is linked (Windows)

## Next Steps

1. **Explore Examples**: Check the `examples/` directory for more complex usage
2. **Read Documentation**: See the full API reference in README.md
3. **Advanced Features**: Try `advanced-features.exe` for macro recording and gaming utilities
4. **Custom Applications**: Start building your own automation tools!

## Getting Help

- Check the main README.md for full API documentation
- Run the integration tests to verify your setup
- Look at example code for usage patterns
- Ensure your device is properly configured

## Quick API Reference

### Basic Operations
```cpp
makcu::Device device;
device.connect();                           // Auto-connect to first device
device.connect("COM3");                     // Connect to specific port

// Keyboard
device.keyPress(makcu::KeyCode::KEY_A);     // Press 'A'
device.typeString("Hello World");           // Type text
device.multiKeyPress({makcu::KeyCode::KEY_LEFT_CTRL, makcu::KeyCode::KEY_C}); // Ctrl+C

// Mouse
device.mouseMove(50, 25);                   // Move relatively
device.mouseMoveTo(100, 100);              // Move to absolute position
device.mouseClick(makcu::MouseButton::LEFT); // Click
device.mouseWheel(1);                       // Scroll up

device.disconnect();                        // Disconnect
```

### Monitoring Events
```cpp
device.setMouseButtonCallback([](makcu::MouseButton btn, bool pressed) {
    if (pressed) std::cout << "Button pressed!" << std::endl;
});
```

Happy coding with MAKCU! 🚀
