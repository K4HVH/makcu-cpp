#include "../makcu-cpp/include/makcu.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

// Simple mouse button monitoring example
// This example shows how to monitor mouse button events in real-time

std::atomic<bool> running{true};

void onMouseButton(makcu::MouseButton button, bool isPressed) {
    if (isPressed) {
        std::string buttonName = makcu::mouseButtonToString(button);
        std::cout << "[" << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() 
            << "] Mouse button " << buttonName << " pressed" << std::endl;
        
        // Demonstrate mouse click detection
        switch (button) {
        case makcu::MouseButton::LEFT:
            std::cout << "  -> Left click detected!" << std::endl;
            break;
        case makcu::MouseButton::RIGHT:
            std::cout << "  -> Right click detected!" << std::endl;
            break;
        case makcu::MouseButton::MIDDLE:
            std::cout << "  -> Middle click detected!" << std::endl;
            break;
        case makcu::MouseButton::SIDE4:
            std::cout << "  -> Side button 4 detected!" << std::endl;
            break;
        case makcu::MouseButton::SIDE5:
            std::cout << "  -> Side button 5 detected!" << std::endl;
            // Stop monitoring when side button 5 is pressed
            running = false;
            break;
        }
    }
}

int main() {
    std::cout << "MAKCU Mouse Button Monitor Example\n";
    std::cout << "===================================\n\n";
    std::cout << "This example monitors mouse button events.\n";
    std::cout << "Press Side Button 5 (if available) to exit, or Ctrl+C\n\n";

    try {
        makcu::Device device;
        
        // Find and connect to device
        std::string port = makcu::Device::findFirstDevice();
        if (port.empty()) {
            std::cout << "No MAKCU device found.\n";
            return 1;
        }

        std::cout << "Connecting to device on " << port << "...\n";
        if (!device.connect(port)) {
            std::cout << "Failed to connect to device.\n";
            return 1;
        }

        std::cout << "Connected successfully!\n";
        std::cout << "Monitoring mouse buttons... (move and click your mouse)\n\n";

        // Set up mouse button callback
        device.setMouseButtonCallback(onMouseButton);

        // Monitor until stopped
        while (running && device.isConnected()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::cout << "\nStopping monitor...\n";
        device.disconnect();
        std::cout << "Disconnected.\n";

    } catch (const makcu::MakcuException& e) {
        std::cerr << "MAKCU Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
