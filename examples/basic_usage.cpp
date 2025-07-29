// Basic usage example for MAKCU C++ Library
#include <makcu.h>
#include <iostream>

int main() {
    try {
        // Find and connect to device
        auto devices = makcu::Device::findDevices();
        if (devices.empty()) {
            std::cout << "No MAKCU devices found.\n";
            return 1;
        }

        makcu::Device device;
        if (!device.connect(devices[0].port)) {
            std::cout << "Failed to connect to device.\n";
            return 1;
        }

        std::cout << "Connected to device: " << devices[0].port << "\n";
        std::cout << "Firmware version: " << device.getVersion() << "\n";

        // Basic mouse operations
        device.mouseMove(100, 0);    // Move right
        device.mouseMove(-100, 0);   // Move back left
        device.click(makcu::MouseButton::LEFT);  // Single click
        device.mouseWheel(3);        // Scroll up
        device.mouseWheel(-3);       // Scroll down

        device.disconnect();
        std::cout << "Demo completed successfully!\n";
    }
    catch (const makcu::MakcuException& e) {
        std::cerr << "MAKCU Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}