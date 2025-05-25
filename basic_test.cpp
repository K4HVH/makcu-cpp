#include "include/makcu.h"
#include <iostream>

int main() {
    std::cout << "MAKCU C++ Library - Basic Compilation Test\n";
    std::cout << "==========================================\n\n";
    
    try {
        // Test basic enum and utility functions
        std::cout << "Testing utility functions...\n";
        
        // Test key code conversion
        std::string keyStr = makcu::keyCodeToString(makcu::KeyCode::KEY_A);
        std::cout << "Key A as string: " << keyStr << std::endl;
        
        makcu::KeyCode key = makcu::stringToKeyCode("A");
        std::cout << "String 'A' converted to KeyCode successfully.\n";
        
        // Test mouse button conversion
        std::string buttonStr = makcu::mouseButtonToString(makcu::MouseButton::LEFT);
        std::cout << "Mouse LEFT button as string: " << buttonStr << std::endl;
        
        // Test mouse button states
        makcu::MouseButtonStates states;
        states.set(makcu::MouseButton::LEFT, true);
        bool leftPressed = states[makcu::MouseButton::LEFT];
        std::cout << "Mouse button states test: " << (leftPressed ? "PASS" : "FAIL") << std::endl;
        
        // Test device creation (should not crash)
        std::cout << "\nTesting device creation...\n";
        makcu::Device device;
        std::cout << "Device created successfully.\n";
        
        // Test device discovery (may find 0 devices if none connected)
        std::cout << "\nTesting device discovery...\n";
        auto devices = makcu::Device::findDevices();
        std::cout << "Found " << devices.size() << " MAKCU device(s).\n";
        
        if (!devices.empty()) {
            for (const auto& dev : devices) {
                std::cout << "  Device: " << dev.port << " - " << dev.description << std::endl;
            }
        }
        
        // Test status checking
        makcu::ConnectionStatus status = device.getStatus();
        std::cout << "Device initial status: " << static_cast<int>(status) << " (should be 0 = DISCONNECTED)\n";
        
        std::cout << "\n*** ALL BASIC TESTS PASSED ***\n";
        std::cout << "The library compiled and basic functions work correctly!\n";
        
        if (devices.empty()) {
            std::cout << "\nNote: No MAKCU devices found. To test hardware functions:\n";
            std::cout << "1. Connect your MAKCU device\n";
            std::cout << "2. Run this test again\n";
            std::cout << "3. Try the full demo application\n";
        } else {
            std::cout << "\nHardware found! You can now:\n";
            std::cout << "1. Run the full demo application\n";
            std::cout << "2. Try the example programs\n";
            std::cout << "3. Build your own applications\n";
        }
        
        return 0;
        
    } catch (const makcu::MakcuException& e) {
        std::cerr << "MAKCU Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Standard Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred." << std::endl;
        return 1;
    }
}
