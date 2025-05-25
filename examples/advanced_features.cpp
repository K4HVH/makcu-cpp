#include "../makcu-cpp/include/makcu.h"
#include "../makcu-cpp/include/utilities.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>

// Advanced macro example demonstrating recording and playback
// This example shows how to use the macro recording system

void waitForUser(const std::string& message) {
    std::cout << message << " Press Enter to continue...";
    std::cin.get();
}

void demonstrateMacroRecording(makcu::Device& device) {
    std::cout << "\n--- Macro Recording Demo ---\n";
    std::cout << "This will record your keyboard and mouse actions for 10 seconds.\n";
    waitForUser("Get ready to perform some actions (type, click, move mouse).");

    makcu::MacroRecorder recorder;
    
    // Set up recording options
    recorder.setRecordMouseMovement(true);
    recorder.setMinimumDelay(50);
    recorder.setTimestampMode(true);

    // Set up callbacks to capture events
    device.setMouseButtonCallback([&recorder](makcu::MouseButton button, bool isPressed) {
        recorder.onMouseButton(button, isPressed);
    });

    device.setKeyboardCallback([&recorder](makcu::KeyCode key, bool isPressed) {
        recorder.onKeyboard(key, isPressed);
    });

    std::cout << "Recording started! Perform your actions now...\n";
    recorder.startRecording();

    // Record for 10 seconds
    std::this_thread::sleep_for(std::chrono::seconds(10));

    recorder.stopRecording();
    std::cout << "Recording stopped!\n";

    // Show recording info
    std::cout << "Recorded " << recorder.getActionCount() << " actions.\n";
    std::cout << "Total duration: " << recorder.getTotalDuration().count() << " ms.\n";

    // Save to file
    std::string filename = "recorded_macro.macro";
    if (recorder.saveToFile(filename)) {
        std::cout << "Macro saved to " << filename << "\n";
    }

    waitForUser("Now we'll play back the recorded macro.");

    std::cout << "Playing back macro...\n";
    recorder.playback(device, 1);
    std::cout << "Playback completed!\n";
}

void demonstrateManualMacro(makcu::Device& device) {
    std::cout << "\n--- Manual Macro Creation Demo ---\n";
    waitForUser("This will create a macro programmatically and execute it.");

    makcu::MacroRecorder macro;

    // Create a macro that opens notepad and types a message
    std::cout << "Creating macro: Open Run dialog, launch Notepad, type message...\n";

    // Win + R to open Run dialog
    macro.addMultiKeyPress({makcu::KeyCode::KEY_LEFT_GUI, makcu::KeyCode::KEY_R});
    macro.addDelay(1000);

    // Type "notepad"
    macro.addTypeString("notepad");
    macro.addDelay(500);

    // Press Enter
    macro.addKeyPress(makcu::KeyCode::KEY_ENTER);
    macro.addDelay(2000); // Wait for Notepad to open

    // Type a message
    macro.addTypeString("This message was typed by the MAKCU C++ Library!");
    macro.addDelay(500);

    // Add some formatting
    macro.addKeyPress(makcu::KeyCode::KEY_ENTER);
    macro.addKeyPress(makcu::KeyCode::KEY_ENTER);
    macro.addTypeString("Macro execution completed successfully.");

    std::cout << "Executing macro...\n";
    macro.playback(device, 1);
    std::cout << "Macro execution completed!\n";
}

void demonstrateGamingUtilities(makcu::Device& device) {
    std::cout << "\n--- Gaming Utilities Demo ---\n";
    waitForUser("This will demonstrate gaming-specific features.");

    // Auto-clicker example
    std::cout << "Auto-clicker demo: Clicking left mouse button rapidly for 3 seconds...\n";
    waitForUser("Position your mouse where you want to click.");
    
    makcu::GamingUtilities::autoClick(device, makcu::MouseButton::LEFT, 100, 3000);
    std::cout << "Auto-clicking completed!\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Recoil control pattern demo
    std::cout << "Recoil control demo: Simulating recoil compensation...\n";
    waitForUser("This will move the mouse in a recoil compensation pattern.");

    // Create a simple recoil pattern (moving down and slightly to the right)
    std::vector<std::pair<int32_t, int32_t>> recoilPattern = {
        {0, 2},   {1, 3},   {0, 4},   {-1, 3},  {0, 4},
        {1, 2},   {0, 3},   {-1, 4},  {0, 3},   {1, 2},
        {0, 4},   {-1, 3},  {1, 4},   {0, 2},   {0, 3}
    };

    makcu::GamingUtilities::performRecoilControl(device, recoilPattern, 20);
    std::cout << "Recoil control demo completed!\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Key sequence demo
    std::cout << "Key sequence demo: Performing ability combo...\n";
    waitForUser("This will simulate pressing keys Q, W, E, R in sequence.");

    std::vector<makcu::KeyCode> comboKeys = {
        makcu::KeyCode::KEY_Q,
        makcu::KeyCode::KEY_W,
        makcu::KeyCode::KEY_E,
        makcu::KeyCode::KEY_R
    };

    makcu::GamingUtilities::performKeySequence(device, comboKeys, 200);
    std::cout << "Key sequence demo completed!\n";
}

void demonstrateHumanization(makcu::Device& device) {
    std::cout << "\n--- Humanization Demo ---\n";
    waitForUser("This will demonstrate humanized movement and timing.");

    std::cout << "Performing humanized mouse movements...\n";

    // Move mouse with humanized variance
    for (int i = 0; i < 10; ++i) {
        auto movement = makcu::GamingUtilities::humanizeMovement(20, 0, 3);
        device.mouseMove(movement.first, movement.second);
        
        uint32_t delay = makcu::GamingUtilities::humanizeDelay(100, 30);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    // Return to start position with variance
    for (int i = 0; i < 10; ++i) {
        auto movement = makcu::GamingUtilities::humanizeMovement(-20, 0, 3);
        device.mouseMove(movement.first, movement.second);
        
        uint32_t delay = makcu::GamingUtilities::humanizeDelay(100, 30);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    std::cout << "Humanized movement demo completed!\n";
}

void demonstrateAdvancedFeatures(makcu::Device& device) {
    std::cout << "\n--- Advanced Features Demo ---\n";
    
    // Clipboard operations
    std::cout << "Clipboard demo...\n";
    std::string originalClipboard = makcu::AutomationHelper::getClipboardText();
    std::cout << "Original clipboard: " << originalClipboard << std::endl;
    
    makcu::AutomationHelper::setClipboardText("MAKCU C++ Library Test");
    std::cout << "Set clipboard to: MAKCU C++ Library Test\n";
    
    std::string newClipboard = makcu::AutomationHelper::getClipboardText();
    std::cout << "Current clipboard: " << newClipboard << std::endl;
    
    // Restore original clipboard
    makcu::AutomationHelper::setClipboardText(originalClipboard);

    // Process monitoring
    std::cout << "\nProcess monitoring demo...\n";
    auto processes = makcu::AutomationHelper::getRunningProcesses();
    std::cout << "Found " << processes.size() << " running processes.\n";
    
    // Check for common processes
    std::vector<std::string> commonProcesses = {"notepad.exe", "explorer.exe", "winlogon.exe"};
    for (const auto& process : commonProcesses) {
        bool running = makcu::AutomationHelper::isProcessRunning(process);
        std::cout << process << ": " << (running ? "RUNNING" : "NOT RUNNING") << std::endl;
    }

    // Screen capture demo
    std::cout << "\nScreen capture demo...\n";
    if (makcu::AutomationHelper::captureScreen("screenshot.bmp")) {
        std::cout << "Screenshot saved to screenshot.bmp\n";
    } else {
        std::cout << "Screenshot failed\n";
    }
}

int main() {
    std::cout << "MAKCU Advanced Features Example\n";
    std::cout << "===============================\n\n";
    std::cout << "This example demonstrates advanced features including:\n";
    std::cout << "- Macro recording and playback\n";
    std::cout << "- Gaming utilities\n";
    std::cout << "- Humanization features\n";
    std::cout << "- System automation helpers\n\n";

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

        std::cout << "Connected successfully!\n\n";

        // Menu system
        while (true) {
            std::cout << "\nSelect demo:\n";
            std::cout << "1. Macro Recording\n";
            std::cout << "2. Manual Macro Creation\n";
            std::cout << "3. Gaming Utilities\n";
            std::cout << "4. Humanization Features\n";
            std::cout << "5. Advanced System Features\n";
            std::cout << "6. Exit\n";
            std::cout << "Choice: ";

            int choice;
            std::cin >> choice;
            std::cin.ignore(); // Consume newline

            switch (choice) {
            case 1:
                demonstrateMacroRecording(device);
                break;
            case 2:
                demonstrateManualMacro(device);
                break;
            case 3:
                demonstrateGamingUtilities(device);
                break;
            case 4:
                demonstrateHumanization(device);
                break;
            case 5:
                demonstrateAdvancedFeatures(device);
                break;
            case 6:
                std::cout << "Exiting...\n";
                device.disconnect();
                return 0;
            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
            }
        }

    } catch (const makcu::MakcuException& e) {
        std::cerr << "MAKCU Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
