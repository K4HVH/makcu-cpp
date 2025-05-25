#include "../makcu-cpp/include/makcu.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>

// Keyboard automation example
// This example demonstrates various keyboard automation capabilities

void waitForUser(const std::string& message) {
    std::cout << message << " Press Enter to continue...";
    std::cin.get();
}

void demonstrateBasicTyping(makcu::Device& device) {
    std::cout << "\n--- Basic Typing Demo ---\n";
    waitForUser("This will type some text.");
    
    device.typeString("Hello from MAKCU C++ Library!");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    device.keyPress(makcu::KeyCode::KEY_ENTER);
    device.typeString("This demonstrates basic text typing capabilities.");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void demonstrateKeyboardShortcuts(makcu::Device& device) {
    std::cout << "\n--- Keyboard Shortcuts Demo ---\n";
    waitForUser("This will demonstrate common keyboard shortcuts.");
    
    // Select all (Ctrl+A)
    std::cout << "Performing Ctrl+A (Select All)...\n";
    device.multiKeyPress({makcu::KeyCode::KEY_LEFT_CTRL, makcu::KeyCode::KEY_A});
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Copy (Ctrl+C)
    std::cout << "Performing Ctrl+C (Copy)...\n";
    device.multiKeyPress({makcu::KeyCode::KEY_LEFT_CTRL, makcu::KeyCode::KEY_C});
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Move to end of document (Ctrl+End)
    std::cout << "Performing Ctrl+End (Go to end)...\n";
    device.multiKeyPress({makcu::KeyCode::KEY_LEFT_CTRL, makcu::KeyCode::KEY_END});
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // New line and paste (Ctrl+V)
    device.keyPress(makcu::KeyCode::KEY_ENTER);
    std::cout << "Performing Ctrl+V (Paste)...\n";
    device.multiKeyPress({makcu::KeyCode::KEY_LEFT_CTRL, makcu::KeyCode::KEY_V});
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void demonstrateSpecialKeys(makcu::Device& device) {
    std::cout << "\n--- Special Keys Demo ---\n";
    waitForUser("This will demonstrate special key combinations.");
    
    // Alt+Tab simulation
    std::cout << "Performing Alt+Tab (Switch windows)...\n";
    device.keyDown(makcu::KeyCode::KEY_LEFT_ALT);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    device.keyPress(makcu::KeyCode::KEY_TAB);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    device.keyUp(makcu::KeyCode::KEY_LEFT_ALT);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Windows key + R (Run dialog)
    std::cout << "Performing Win+R (Run dialog)...\n";
    device.multiKeyPress({makcu::KeyCode::KEY_LEFT_GUI, makcu::KeyCode::KEY_R});
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Type notepad and press enter
    device.typeString("notepad");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    device.keyPress(makcu::KeyCode::KEY_ENTER);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // Wait for notepad to open
}

void demonstrateAdvancedTyping(makcu::Device& device) {
    std::cout << "\n--- Advanced Typing Demo ---\n";
    waitForUser("This will demonstrate advanced typing with timing.");
    
    // Type with human-like delays
    std::string text = "This text is typed with realistic delays...";
    for (char c : text) {
        if (c == ' ') {
            device.keyPress(makcu::KeyCode::KEY_SPACEBAR, 50 + (rand() % 100));
        } else if (c >= 'a' && c <= 'z') {
            makcu::KeyCode key = static_cast<makcu::KeyCode>(
                static_cast<int>(makcu::KeyCode::KEY_A) + (c - 'a'));
            device.keyPress(key, 80 + (rand() % 120)); // Random delay between 80-200ms
        } else if (c == '.') {
            device.keyPress(makcu::KeyCode::KEY_PERIOD, 100 + (rand() % 150));
        }
        
        // Add small random delay between keystrokes
        std::this_thread::sleep_for(std::chrono::milliseconds(50 + (rand() % 100)));
    }
    
    device.keyPress(makcu::KeyCode::KEY_ENTER);
}

void demonstrateFunctionKeys(makcu::Device& device) {
    std::cout << "\n--- Function Keys Demo ---\n";
    waitForUser("This will demonstrate function key usage.");
    
    // F5 (Refresh)
    std::cout << "Pressing F5 (Refresh)...\n";
    device.keyPress(makcu::KeyCode::KEY_F5);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // F11 (Full screen toggle)
    std::cout << "Pressing F11 (Toggle fullscreen)...\n";
    device.keyPress(makcu::KeyCode::KEY_F11);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // F11 again to return from fullscreen
    std::cout << "Pressing F11 again (Return from fullscreen)...\n";
    device.keyPress(makcu::KeyCode::KEY_F11);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main() {
    std::cout << "MAKCU Keyboard Automation Example\n";
    std::cout << "==================================\n\n";
    std::cout << "This example demonstrates various keyboard automation capabilities.\n";
    std::cout << "Make sure you have a text editor or document open to see the effects.\n\n";

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

        // Wait for user to be ready
        waitForUser("Please open a text editor (like Notepad) and position your cursor.");

        // Run demonstrations
        demonstrateBasicTyping(device);
        demonstrateKeyboardShortcuts(device);
        demonstrateAdvancedTyping(device);
        demonstrateFunctionKeys(device);
        demonstrateSpecialKeys(device);

        std::cout << "\nAll demonstrations completed!\n";
        device.disconnect();
        std::cout << "Disconnected from device.\n";

    } catch (const makcu::MakcuException& e) {
        std::cerr << "MAKCU Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
