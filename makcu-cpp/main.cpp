#include "include/makcu.h"
#include "include/serialport.h"
#include <iostream>
#include <thread>
#include <chrono>

void mouseButtonCallback(makcu::MouseButton button, bool isPressed) {
    std::string buttonName = makcu::mouseButtonToString(button);
    std::string state = isPressed ? "PRESSED" : "RELEASED";
    std::cout << "Mouse button " << buttonName << " " << state << std::endl;
}

int main() {
    std::cout << "MAKCU C++ Mouse Controller Demo\n";
    std::cout << "================================\n\n";

    try {
        // Create device instance
        makcu::Device device;

        // Find available devices
        std::cout << "Scanning for MAKCU devices...\n";
        auto devices = makcu::Device::findDevices();

        if (devices.empty()) {
            std::cout << "No MAKCU devices found. Please connect your device and try again.\n";
            return 1;
        }

        std::cout << "Found " << devices.size() << " device(s):\n";
        for (const auto& dev : devices) {
            std::cout << "  Port: " << dev.port << ", Description: " << dev.description << "\n";
        }

        // Connect to the first device
        std::cout << "\nConnecting to " << devices[0].port << "...\n";

        if (!device.connect(devices[0].port)) {
            std::cout << "Failed to connect to device.\n";
            return 1;
        }

        std::cout << "Successfully connected!\n";

        // Get device information
        auto deviceInfo = device.getDeviceInfo();
        std::cout << "Device Info:\n";
        std::cout << "  Port: " << deviceInfo.port << "\n";
        std::cout << "  VID: 0x" << std::hex << deviceInfo.vid << "\n";
        std::cout << "  PID: 0x" << std::hex << deviceInfo.pid << std::dec << "\n";
        std::cout << "  Version: " << device.getVersion() << "\n";

        // Set up callback for real-time monitoring
        //device.setMouseButtonCallback(mouseButtonCallback);

        std::cout << "\nDemonstrating MAKCU mouse functionality:\n";

        // Test basic mouse movement
        std::cout << "\n1. Basic mouse movement...\n";
        device.mouseMove(50, 0);    // Move right
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        device.mouseMove(-50, 0);   // Move left
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Test smooth movement (v3.2 feature)
        std::cout << "\n2. Smooth curved movement...\n";
        device.mouseMoveSmooth(100, 100, 20);  // Smooth curve over 20 segments
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Test Bezier curve movement (v3.2 feature)
        std::cout << "\n3. Bezier curve movement...\n";
        device.mouseMoveBezier(-100, -100, 15, -50, -25);  // Custom Bezier curve
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Test mouse wheel
        std::cout << "\n4. Mouse wheel scrolling...\n";
        device.mouseWheel(3);  // Scroll up
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        device.mouseWheel(-3); // Scroll down
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Test mouse button states
        std::cout << "\n5. Mouse button states:\n";
        std::cout << "  Left button: " << (device.mouseButtonState(makcu::MouseButton::LEFT) ? "DOWN" : "UP") << "\n";
        std::cout << "  Right button: " << (device.mouseButtonState(makcu::MouseButton::RIGHT) ? "DOWN" : "UP") << "\n";
        std::cout << "  Middle button: " << (device.mouseButtonState(makcu::MouseButton::MIDDLE) ? "DOWN" : "UP") << "\n";

        // Test mouse locking
        std::cout << "\n6. Testing mouse locking (will lock X movement for 3 seconds)...\n";
        device.lockMouseX(true);
        std::cout << "  X movement locked - try moving your mouse left/right\n";
        std::this_thread::sleep_for(std::chrono::seconds(3));
        device.lockMouseX(false);
        std::cout << "  X movement unlocked\n";

        // Test serial spoofing info
        std::cout << "\n7. Mouse serial info:\n";
        std::cout << "  Serial: " << device.getMouseSerial() << "\n";

        // Monitor for button presses
        std::cout << "\nMonitoring mouse button events for 10 seconds...\n";
        std::cout << "Press any mouse buttons to see real-time detection.\n";

        for (int i = 0; i < 100 && device.isConnected(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::cout << "\nDisconnecting...\n";
        device.disconnect();
        std::cout << "Demo completed successfully.\n";

    }
    catch (const makcu::MakcuException& e) {
        std::cerr << "MAKCU Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}