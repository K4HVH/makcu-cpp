# MAKCU C++ Library

**A professional C++ wrapper for MAKCU devices using the KMBOX B+ pro command set.**

## ⚡ Quick Start (Fixed Version)

**If you experienced compilation errors, this version is completely rewritten and should compile cleanly!**

### 🔧 **Immediate Test (30 seconds)**

**Option 1: Visual Studio**
```bash
1. Open "makcu-basic-test.vcxproj"
2. Press F7 to build
3. Press F5 to run
4. Should show "ALL BASIC TESTS PASSED"
```

**Option 2: Command Line**
```bash
1. Open "x64 Native Tools Command Prompt" for Visual Studio  
2. Navigate to project folder
3. Run: quick_test.bat
4. Should compile and run successfully
```

### 🎯 **Expected Output**
```
MAKCU C++ Library - Basic Compilation Test
==========================================

Testing utility functions...
Key A as string: A
Mouse LEFT button as string: LEFT
Mouse button states test: PASS

*** ALL BASIC TESTS PASSED ***
The library compiled and basic functions work correctly!
```

---

### Basic Usage

```cpp
#include "include/makcu.h"
#include <iostream>

int main() {
    try {
        // Create device instance
        makcu::Device device;
        
        // Find and connect to device
        std::string port = makcu::Device::findFirstDevice();
        if (port.empty()) {
            std::cout << "No MAKCU device found.\n";
            return 1;
        }
        
        if (!device.connect(port)) {
            std::cout << "Failed to connect to device.\n";
            return 1;
        }
        
        std::cout << "Connected successfully!\n";
        
        // Type some text
        device.typeString("Hello from MAKCU!");
        
        // Press Enter
        device.keyPress(makcu::KeyCode::KEY_ENTER);
        
        // Move mouse cursor
        device.mouseMove(100, 50);
        
        // Left click
        device.mouseClick(makcu::MouseButton::LEFT);
        
        // Disconnect
        device.disconnect();
        
    } catch (const makcu::MakcuException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### Real-time Input Monitoring

```cpp
#include "include/makcu.h"
#include <iostream>

void onMouseButton(makcu::MouseButton button, bool isPressed) {
    if (isPressed) {
        std::cout << "Mouse button " << makcu::mouseButtonToString(button) 
                  << " pressed" << std::endl;
    }
}

void onKeyboard(makcu::KeyCode key, bool isPressed) {
    if (isPressed) {
        std::cout << "Key " << makcu::keyCodeToString(key) 
                  << " pressed" << std::endl;
    }
}

int main() {
    makcu::Device device;
    
    if (device.connect()) {
        // Set up callbacks
        device.setMouseButtonCallback(onMouseButton);
        device.setKeyboardCallback(onKeyboard);
        
        // Monitor for 10 seconds
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        device.disconnect();
    }
    
    return 0;
}
```

## API Reference

### Device Management

#### `makcu::Device`

Main class for device control and automation.

```cpp
// Construction and connection
Device();
~Device();

// Device discovery
static std::vector<DeviceInfo> findDevices();
static std::string findFirstDevice();

// Connection management
bool connect(const std::string& port = "");
void disconnect();
bool isConnected() const;
ConnectionStatus getStatus() const;

// Device information
DeviceInfo getDeviceInfo() const;
std::string getVersion() const;
std::string getSerialNumber() const;
```

### Keyboard Control

```cpp
// Basic key operations
bool keyDown(KeyCode key);
bool keyUp(KeyCode key);
bool keyPress(KeyCode key, uint32_t duration_ms = 0);

// Multi-key operations
bool multiKeyDown(const std::vector<KeyCode>& keys);
bool multiKeyUp(const std::vector<KeyCode>& keys);
bool multiKeyPress(const std::vector<KeyCode>& keys, uint32_t duration_ms = 0);

// Text input
bool typeString(const std::string& text);

// State checking
bool isKeyDown(KeyCode key);
```

### Mouse Control

```cpp
// Button operations
bool mouseDown(MouseButton button);
bool mouseUp(MouseButton button);
bool mouseClick(MouseButton button, uint32_t count = 1);

// Movement
bool mouseMove(int32_t x, int32_t y);         // Relative movement
bool mouseMoveTo(int32_t x, int32_t y);       // Absolute movement
bool mouseWheel(int32_t delta);

// Advanced positioning
bool mouseSetPosition(int32_t x, int32_t y);
std::pair<int32_t, int32_t> mouseGetPosition();
bool mouseCalibrate();
bool mouseSetScreenBounds(int32_t width, int32_t height);

// State monitoring
MouseButtonStates getMouseButtonStates();
```

### Event Callbacks

```cpp
// Callback types
using MouseButtonCallback = std::function<void(MouseButton, bool)>;
using KeyboardCallback = std::function<void(KeyCode, bool)>;

// Callback registration
void setMouseButtonCallback(MouseButtonCallback callback);
void setKeyboardCallback(KeyboardCallback callback);
```

### Utility Functions

```cpp
// String conversions
std::string keyCodeToString(KeyCode key);
KeyCode stringToKeyCode(const std::string& keyName);
std::string mouseButtonToString(MouseButton button);

// Device control
bool delay(uint32_t milliseconds);
bool sendRawCommand(const std::string& command);
std::string receiveRawResponse();
```

## Key Codes

The library includes comprehensive key code definitions based on the HID specification:

```cpp
// Letters
KeyCode::KEY_A through KeyCode::KEY_Z

// Numbers  
KeyCode::KEY_1 through KeyCode::KEY_0

// Function keys
KeyCode::KEY_F1 through KeyCode::KEY_F12

// Special keys
KeyCode::KEY_ENTER, KeyCode::KEY_ESCAPE, KeyCode::KEY_SPACEBAR
KeyCode::KEY_LEFT_CTRL, KeyCode::KEY_LEFT_ALT, KeyCode::KEY_LEFT_SHIFT
KeyCode::KEY_ARROW_UP, KeyCode::KEY_ARROW_DOWN, etc.

// And many more...
```

## Mouse Buttons

```cpp
enum class MouseButton {
    LEFT = 0,    // Left mouse button
    RIGHT = 1,   // Right mouse button  
    MIDDLE = 2,  // Middle mouse button (wheel click)
    SIDE4 = 3,   // Side button 4
    SIDE5 = 4    // Side button 5
};
```

## Examples

The `examples/` directory contains several demonstration programs:

- **`mouse_monitor.cpp`**: Real-time mouse button event monitoring
- **`keyboard_automation.cpp`**: Comprehensive keyboard automation demos
- **`mouse_automation.cpp`**: Advanced mouse movement and control patterns

To build and run examples:

1. Copy the example file to your project
2. Include the necessary headers
3. Link against the MAKCU library
4. Compile and run

## Error Handling

The library uses exceptions for error handling:

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

## Device Detection and Setup

### Automatic Detection

The library automatically detects MAKCU devices by scanning for:
- VID: 0x1A86 (WinChipHead)
- PID: 0x55D3 (MAKCU device identifier)
- Device description containing "USB-Enhanced-SERIAL CH343"

### Manual Port Specification

```cpp
makcu::Device device;
device.connect("COM3");  // Connect to specific port
```

### COM Port Management

The included COM port changer tool allows you to:
- Change device friendly names
- Spoof device identity for compatibility
- Restore original device names

## Communication Protocol

The library handles the low-level communication protocol:

1. **Initial Connection**: 115200 baud rate
2. **Baud Rate Switch**: Automatically switches to 4MHz for high performance
3. **Command Format**: KMBOX command set over serial
4. **Response Parsing**: Automatic response handling and parsing

## Threading Considerations

The library is designed to be thread-safe:

- Device operations are protected by internal mutexes
- Callback functions are called from a dedicated monitoring thread
- Multiple threads can safely call device methods simultaneously

## Performance Notes

- **High Speed Communication**: 4MHz serial communication for minimal latency
- **Efficient Protocol**: Optimized command structure for fast execution
- **Hardware-level Control**: Direct hardware control bypasses software limitations
- **Real-time Monitoring**: Dedicated thread for input event processing

## Troubleshooting

### Common Issues

1. **Device Not Found**
   - Ensure device is properly connected
   - Check Device Manager for COM port assignment
   - Verify device drivers are installed

2. **Connection Failed**
   - Try different COM ports
   - Ensure no other applications are using the device
   - Check device power supply

3. **Commands Not Working**
   - Verify device connection status
   - Check for proper command syntax
   - Ensure target application has focus

### Debug Output

Enable debug output by checking connection status:

```cpp
if (device.getStatus() == makcu::ConnectionStatus::ERROR) {
    std::cout << "Connection failed" << std::endl;
}
```

## Compatibility

- **Windows**: Windows 10/11 (primary support)
- **Visual Studio**: 2019 or later
- **C++ Standard**: C++17 or later
- **Architecture**: x86 and x64

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## Support

For support and questions:
1. Check the examples directory for usage patterns
2. Review the API documentation above
3. Open an issue on the project repository

## Acknowledgments

Based on the KMBOX B+ pro command set and inspired by the Python implementation examples provided with MAKCU devices.
