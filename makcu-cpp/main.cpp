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

void keyboardCallback(makcu::KeyCode key, bool isPressed) {
    std::string keyName = makcu::keyCodeToString(key);
    std::string state = isPressed ? "PRESSED" : "RELEASED";
    std::cout << "Key " << keyName << " " << state << std::endl;
}

int main() {
    std::cout << "MAKCU C++ Library Demo\n";
    std::cout << "======================\n\n";

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

        // Set up callbacks for real-time monitoring
        device.setMouseButtonCallback(mouseButtonCallback);
        device.setKeyboardCallback(keyboardCallback);

        std::cout << "\nMonitoring mouse and keyboard events. Press Ctrl+C to exit.\n";
        std::cout << "Try the following demo commands:\n";
        std::cout << "  - Mouse buttons will be detected automatically\n";
        std::cout << "  - Demonstrating programmatic control...\n\n";

        // Demonstrate mouse control
        std::cout << "Demo: Moving mouse cursor...\n";
        device.mouseMove(100, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        device.mouseMove(-100, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Demonstrate keyboard control
        std::cout << "Demo: Typing text...\n";
        device.typeString("Hello from MAKCU C++ Library!");
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Demonstrate key combinations
        std::cout << "Demo: Key combinations...\n";
        std::vector<makcu::KeyCode> keys = {makcu::KeyCode::KEY_LEFT_CTRL, makcu::KeyCode::KEY_A};
        device.multiKeyPress(keys);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Monitor for 30 seconds or until interrupted
        std::cout << "\nListening for mouse/keyboard events for 30 seconds...\n";
        for (int i = 0; i < 300 && device.isConnected(); ++i) {
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
