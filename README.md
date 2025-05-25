# MAKCU C++ Library

**A professional C++ wrapper for MAKCU mouse controllers with advanced automation capabilities.**

## Overview

The MAKCU C++ Library provides a high-level interface for controlling MAKCU mouse devices. These devices act as hardware-level mouse controllers, offering precise movement control, button automation, and advanced features like input masking and real-time monitoring.

## Features

- **Hardware-Level Mouse Control**: Direct control over mouse buttons, movement, and wheel
- **Advanced Movement**: Instant, smooth curved, and Bezier curve movement patterns
- **Input Masking**: Block physical mouse input while maintaining software control
- **Real-Time Monitoring**: Live button state detection and event callbacks
- **High-Speed Communication**: 4MHz serial communication for minimal latency
- **Button State Management**: Comprehensive button state tracking and control
- **Serial Spoofing**: Change device identity for compatibility (v3.2)

## Device Compatibility

- **Device Type**: MAKCU Mouse Controller
- **VID/PID**: 0x1A86/0x55D3
- **Description**: "USB-Enhanced-SERIAL CH343"
- **Communication**: Serial over USB (115200 → 4MHz auto-switching)
- **Platform**: Windows (Visual Studio 2019+, C++17)

## Quick Start

### Installation

1. Clone or download the repository
2. Open `makcu-cpp.sln` in Visual Studio
3. Build the project (F7)
4. Run the example (F5)

### Basic Usage

```cpp
#include "include/makcu.h"
#include <iostream>

int main() {
    try {
        // Create and connect to device
        makcu::Device device;
        
        std::string port = makcu::Device::findFirstDevice();
        if (port.empty()) {
            std::cout << "No MAKCU device found.\n";
            return 1;
        }
        
        if (!device.connect(port)) {
            std::cout << "Failed to connect.\n";
            return 1;
        }
        
        std::cout << "Connected to " << port << "\n";
        
        // Basic mouse control
        device.mouseMove(100, 50);              // Move cursor
        device.mouseDown(makcu::MouseButton::LEFT);   // Press left button
        device.mouseUp(makcu::MouseButton::LEFT);     // Release left button
        device.mouseWheel(3);                   // Scroll up
        
        device.disconnect();
        
    } catch (const makcu::MakcuException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## API Reference

### Device Management

```cpp
class Device {
public:
    // Connection
    static std::vector<DeviceInfo> findDevices();
    static std::string findFirstDevice();
    bool connect(const std::string& port = "");
    void disconnect();
    bool isConnected() const;
    
    // Device information
    DeviceInfo getDeviceInfo() const;
    std::string getVersion() const;
    ConnectionStatus getStatus() const;
};
```

### Mouse Control

#### Button Operations

```cpp
// Button control
bool mouseDown(MouseButton button);
bool mouseUp(MouseButton button);
bool mouseButtonState(MouseButton button);  // Get current state

// Supported buttons
enum class MouseButton {
    LEFT = 0,    // Left mouse button
    RIGHT = 1,   // Right mouse button  
    MIDDLE = 2,  // Middle button (wheel click)
    SIDE1 = 3,   // Side button 1
    SIDE2 = 4    // Side button 2
};
```

#### Movement Control

```cpp
// Movement options
bool mouseMove(int32_t x, int32_t y);                          // Instant movement
bool mouseMoveSmooth(int32_t x, int32_t y, uint32_t segments); // Smooth curve
bool mouseMoveBezier(int32_t x, int32_t y, uint32_t segments,  // Custom Bezier
                     int32_t ctrl_x, int32_t ctrl_y);

// Mouse wheel
bool mouseWheel(int32_t delta);  // Positive = up, negative = down
```

#### Input Masking

```cpp
// Lock physical mouse input (software retains control)
bool lockMouseX(bool lock = true);      // Block X-axis movement
bool lockMouseY(bool lock = true);      // Block Y-axis movement
bool lockMouseLeft(bool lock = true);   // Block left button
bool lockMouseMiddle(bool lock = true); // Block middle button
bool lockMouseRight(bool lock = true);  // Block right button

// Check lock states
bool isMouseXLocked();
bool isMouseYLocked();
bool isMouseLeftLocked();
// ... etc
```

#### Input Monitoring

```cpp
// Button monitoring (v3.2 bitmask API)
bool enableButtonMonitoring(bool enable = true);
uint8_t getButtonMask();  // Returns bitmask of all button states

// Event callbacks
typedef std::function<void(MouseButton, bool)> MouseButtonCallback;
void setMouseButtonCallback(MouseButtonCallback callback);
```

#### Advanced Features

```cpp
// Capture physical input (when locked)
uint8_t catchMouseLeft();   // Get captured left button presses
uint8_t catchMouseRight();  // Get captured right button presses
uint8_t catchMouseMiddle(); // Get captured middle button presses

// Serial spoofing (v3.2)
std::string getMouseSerial();
bool setMouseSerial(const std::string& serial);
bool resetMouseSerial();
```

## Movement Patterns

### Instant Movement

```cpp
device.mouseMove(100, 100);  // Move 100 pixels right and down instantly
```

### Smooth Curved Movement (v3.2)

```cpp
// Create randomized smooth curve over 20 segments (20ms duration)
device.mouseMoveSmooth(200, 150, 20);
```

### Bezier Curve Movement (v3.2)

```cpp
// Custom Bezier curve with control point
device.mouseMoveBezier(300, 200, 15, 150, 100);
//                     end_x, end_y, segments, ctrl_x, ctrl_y
```

## Real-Time Monitoring

```cpp
void onMouseButton(makcu::MouseButton button, bool isPressed) {
    std::string action = isPressed ? "PRESSED" : "RELEASED";
    std::cout << "Button " << makcu::mouseButtonToString(button) 
              << " " << action << std::endl;
}

int main() {
    makcu::Device device;
    device.connect();
    
    // Set up real-time monitoring
    device.setMouseButtonCallback(onMouseButton);
    
    // Monitor for 10 seconds
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    device.disconnect();
    return 0;
}
```

## Button State Bitmask

The device provides a bitmask for all button states:

```cpp
uint8_t mask = device.getButtonMask();

// Check individual buttons
bool leftPressed = (mask & (1 << 0)) != 0;   // Bit 0 = Left
bool rightPressed = (mask & (1 << 1)) != 0;  // Bit 1 = Right  
bool middlePressed = (mask & (1 << 2)) != 0; // Bit 2 = Middle
bool side1Pressed = (mask & (1 << 3)) != 0;  // Bit 3 = Side1
bool side2Pressed = (mask & (1 << 4)) != 0;  // Bit 4 = Side2
```

## Error Handling

```cpp
try {
    device.connect();
    // ... operations
} catch (const makcu::ConnectionException& e) {
    std::cerr << "Connection error: " << e.what() << std::endl;
} catch (const makcu::CommandException& e) {
    std::cerr << "Command error: " << e.what() << std::endl;
} catch (const makcu::MakcuException& e) {
    std::cerr << "MAKCU error: " << e.what() << std::endl;
}
```

## Advanced Use Cases

### Gaming Automation

```cpp
// Precise recoil control
device.mouseMoveSmooth(0, 10, 5);  // Smooth downward movement

// Rapid clicking with human-like variation
for (int i = 0; i < 10; ++i) {
    device.mouseDown(makcu::MouseButton::LEFT);
    std::this_thread::sleep_for(std::chrono::milliseconds(50 + rand() % 20));
    device.mouseUp(makcu::MouseButton::LEFT);
    std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand() % 50));
}
```

### Input Isolation

```cpp
// Lock physical mouse, maintain software control
device.lockMouseX(true);
device.lockMouseY(true);
device.lockMouseLeft(true);

// Software can still move and click
device.mouseMove(100, 100);
device.mouseDown(makcu::MouseButton::LEFT);

// Unlock when done
device.lockMouseX(false);
device.lockMouseY(false);
device.lockMouseLeft(false);
```

## Troubleshooting

### Device Not Found

- Ensure device is connected via USB
- Check Device Manager for COM port assignment
- Verify device description contains "USB-Enhanced-SERIAL CH343"

### Connection Failed

- Try different COM ports if auto-detection fails
- Ensure no other applications are using the device
- Check device power and USB connection

### Commands Not Working

- Verify device connection with `device.isConnected()`
- Check that target application has focus
- Ensure proper command syntax

## Technical Details

- **Communication Protocol**: Custom serial protocol over USB
- **Baud Rate**: 115200 (initial) → 4,000,000 (operational)
- **Response Time**: Sub-millisecond command execution
- **Threading**: Thread-safe with internal synchronization
- **Memory**: Minimal footprint with efficient resource management

## License

GNU GPLv3 License - see LICENSE file for details.

## Support

For support and questions:

1. Check the API documentation above
2. Review the example implementation in `main.cpp`
3. Open an issue on the project repository

## Acknowledgments

Based on the MAKCU device command set and inspired by the Python implementation examples.
