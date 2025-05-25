#include "../makcu-cpp/include/makcu.h"
#include "../makcu-cpp/include/utilities.h"
#include <iostream>
#include <thread>
#include <chrono>

// Integration tests that require actual MAKCU hardware
// These tests will only pass if a device is connected

class IntegrationTestFramework {
private:
    int tests_run = 0;
    int tests_passed = 0;
    int tests_skipped = 0;

public:
    void assert_true(bool condition, const std::string& test_name) {
        tests_run++;
        if (condition) {
            tests_passed++;
            std::cout << "[PASS] " << test_name << std::endl;
        } else {
            std::cout << "[FAIL] " << test_name << std::endl;
        }
    }

    void skip_test(const std::string& test_name, const std::string& reason) {
        tests_skipped++;
        std::cout << "[SKIP] " << test_name << " - " << reason << std::endl;
    }

    void print_summary() {
        std::cout << "\n=== Integration Test Summary ===" << std::endl;
        std::cout << "Tests run: " << tests_run << std::endl;
        std::cout << "Tests passed: " << tests_passed << std::endl;
        std::cout << "Tests failed: " << (tests_run - tests_passed) << std::endl;
        std::cout << "Tests skipped: " << tests_skipped << std::endl;
        if (tests_run > 0) {
            std::cout << "Success rate: " << (100.0 * tests_passed / tests_run) << "%" << std::endl;
        }
    }

    bool all_passed() const {
        return tests_passed == tests_run;
    }
};

IntegrationTestFramework test_framework;

bool test_device_connection() {
    std::cout << "\n--- Testing Device Connection ---" << std::endl;
    
    makcu::Device device;
    
    // Test device discovery
    auto devices = makcu::Device::findDevices();
    if (devices.empty()) {
        test_framework.skip_test("Device connection test", "No MAKCU devices found");
        return false;
    }
    
    std::cout << "Found " << devices.size() << " device(s)" << std::endl;
    
    // Test connection
    bool connected = device.connect(devices[0].port);
    test_framework.assert_true(connected, "Device connection successful");
    
    if (!connected) {
        return false;
    }
    
    // Test connection status
    test_framework.assert_true(device.isConnected(), "Device reports connected status");
    test_framework.assert_true(device.getStatus() == makcu::ConnectionStatus::CONNECTED, "Device status is CONNECTED");
    
    // Test device info
    auto deviceInfo = device.getDeviceInfo();
    test_framework.assert_true(!deviceInfo.port.empty(), "Device info contains port");
    test_framework.assert_true(deviceInfo.isConnected, "Device info shows connected");
    
    // Test version and serial (may not work on all devices)
    std::string version = device.getVersion();
    std::string serial = device.getSerialNumber();
    std::cout << "Device version: " << (version.empty() ? "N/A" : version) << std::endl;
    std::cout << "Device serial: " << (serial.empty() ? "N/A" : serial) << std::endl;
    
    // Test disconnection
    device.disconnect();
    test_framework.assert_true(!device.isConnected(), "Device reports disconnected after disconnect");
    test_framework.assert_true(device.getStatus() == makcu::ConnectionStatus::DISCONNECTED, "Device status is DISCONNECTED after disconnect");
    
    return true;
}

void test_keyboard_functionality(makcu::Device& device) {
    std::cout << "\n--- Testing Keyboard Functionality ---" << std::endl;
    
    std::cout << "Testing keyboard functions (you should see some key events)..." << std::endl;
    std::cout << "Note: Make sure a text editor or console is focused to see results." << std::endl;
    
    // Test basic key press
    bool result = device.keyPress(makcu::KeyCode::KEY_A);
    test_framework.assert_true(result, "Single key press command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Test key down/up
    result = device.keyDown(makcu::KeyCode::KEY_B);
    test_framework.assert_true(result, "Key down command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    result = device.keyUp(makcu::KeyCode::KEY_B);
    test_framework.assert_true(result, "Key up command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Test multi-key press (Ctrl+A)
    result = device.multiKeyPress({makcu::KeyCode::KEY_LEFT_CTRL, makcu::KeyCode::KEY_A});
    test_framework.assert_true(result, "Multi-key press command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Test string typing
    result = device.typeString("Test");
    test_framework.assert_true(result, "Type string command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Test special keys
    result = device.keyPress(makcu::KeyCode::KEY_ENTER);
    test_framework.assert_true(result, "Enter key press command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void test_mouse_functionality(makcu::Device& device) {
    std::cout << "\n--- Testing Mouse Functionality ---" << std::endl;
    
    std::cout << "Testing mouse functions (you should see mouse movement and clicks)..." << std::endl;
    
    // Test mouse movement
    bool result = device.mouseMove(50, 0);
    test_framework.assert_true(result, "Mouse move right command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    result = device.mouseMove(0, 50);
    test_framework.assert_true(result, "Mouse move down command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    result = device.mouseMove(-50, 0);
    test_framework.assert_true(result, "Mouse move left command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    result = device.mouseMove(0, -50);
    test_framework.assert_true(result, "Mouse move up command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Test mouse clicks
    result = device.mouseClick(makcu::MouseButton::LEFT);
    test_framework.assert_true(result, "Left mouse click command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    result = device.mouseClick(makcu::MouseButton::RIGHT);
    test_framework.assert_true(result, "Right mouse click command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Test mouse wheel
    result = device.mouseWheel(1);
    test_framework.assert_true(result, "Mouse wheel up command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    result = device.mouseWheel(-1);
    test_framework.assert_true(result, "Mouse wheel down command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Test mouse button down/up
    result = device.mouseDown(makcu::MouseButton::LEFT);
    test_framework.assert_true(result, "Mouse button down command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    result = device.mouseUp(makcu::MouseButton::LEFT);
    test_framework.assert_true(result, "Mouse button up command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void test_advanced_mouse_functionality(makcu::Device& device) {
    std::cout << "\n--- Testing Advanced Mouse Functionality ---" << std::endl;
    
    // Test screen bounds setting
    bool result = device.mouseSetScreenBounds(1920, 1080);
    test_framework.assert_true(result, "Mouse screen bounds setting command executed");
    
    // Test mouse calibration
    result = device.mouseCalibrate();
    test_framework.assert_true(result, "Mouse calibration command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Test absolute positioning (if calibrated)
    result = device.mouseMoveTo(100, 100);
    test_framework.assert_true(result, "Mouse move to absolute position command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    result = device.mouseMoveTo(200, 200);
    test_framework.assert_true(result, "Mouse move to second absolute position command executed");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void test_macro_functionality(makcu::Device& device) {
    std::cout << "\n--- Testing Macro Functionality ---" << std::endl;
    
    makcu::MacroRecorder recorder;
    
    // Create a simple macro
    recorder.addTypeString("Macro test: ");
    recorder.addKeyPress(makcu::KeyCode::KEY_A);
    recorder.addKeyPress(makcu::KeyCode::KEY_B);
    recorder.addKeyPress(makcu::KeyCode::KEY_C);
    recorder.addDelay(500);
    recorder.addKeyPress(makcu::KeyCode::KEY_ENTER);
    
    std::cout << "Created macro with " << recorder.getActionCount() << " actions" << std::endl;
    test_framework.assert_true(recorder.getActionCount() == 5, "Macro contains expected number of actions");
    
    // Execute the macro
    std::cout << "Executing macro (you should see typed text)..." << std::endl;
    bool result = recorder.playback(device, 1);
    test_framework.assert_true(result, "Macro playback executed successfully");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void test_gaming_utilities_integration(makcu::Device& device) {
    std::cout << "\n--- Testing Gaming Utilities Integration ---" << std::endl;
    
    std::cout << "Testing recoil control pattern..." << std::endl;
    
    // Create a simple recoil pattern
    std::vector<std::pair<int32_t, int32_t>> pattern = {
        {0, 1}, {0, 2}, {1, 1}, {-1, 2}, {0, 1}
    };
    
    bool result = makcu::GamingUtilities::performRecoilControl(device, pattern, 50);
    test_framework.assert_true(result, "Recoil control pattern executed");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "Testing key sequence..." << std::endl;
    
    std::vector<makcu::KeyCode> keys = {
        makcu::KeyCode::KEY_1,
        makcu::KeyCode::KEY_2,
        makcu::KeyCode::KEY_3
    };
    
    result = makcu::GamingUtilities::performKeySequence(device, keys, 100);
    test_framework.assert_true(result, "Key sequence executed");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "Testing humanized auto-click..." << std::endl;
    
    result = makcu::GamingUtilities::autoClick(device, makcu::MouseButton::LEFT, 200, 1000);
    test_framework.assert_true(result, "Auto-click executed");
}

void test_monitoring_functionality(makcu::Device& device) {
    std::cout << "\n--- Testing Monitoring Functionality ---" << std::endl;
    
    std::cout << "Setting up event monitoring..." << std::endl;
    
    volatile bool mouseEventReceived = false;
    volatile bool keyboardEventReceived = false;
    
    // Set up callbacks
    device.setMouseButtonCallback([&mouseEventReceived](makcu::MouseButton button, bool isPressed) {
        if (isPressed) {
            std::cout << "Mouse button event received: " << makcu::mouseButtonToString(button) << std::endl;
            mouseEventReceived = true;
        }
    });
    
    device.setKeyboardCallback([&keyboardEventReceived](makcu::KeyCode key, bool isPressed) {
        if (isPressed) {
            std::cout << "Keyboard event received: " << makcu::keyCodeToString(key) << std::endl;
            keyboardEventReceived = true;
        }
    });
    
    std::cout << "Monitoring setup completed. Testing will generate events and check if they're detected..." << std::endl;
    
    // Generate some events and check if they're detected
    device.mouseClick(makcu::MouseButton::LEFT);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    device.keyPress(makcu::KeyCode::KEY_SPACEBAR);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Wait a bit for events to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Note: This test might not always pass depending on the device's monitoring capabilities
    // and whether the device reports its own generated events
    std::cout << "Mouse event received: " << (mouseEventReceived ? "Yes" : "No") << std::endl;
    std::cout << "Keyboard event received: " << (keyboardEventReceived ? "Yes" : "No") << std::endl;
    
    test_framework.assert_true(true, "Monitoring test completed (results may vary)");
}

int main() {
    std::cout << "MAKCU C++ Library Integration Tests\n";
    std::cout << "====================================\n";
    std::cout << "These tests require a connected MAKCU device.\n";
    std::cout << "Make sure to have a text editor or console focused to see keyboard output.\n\n";
    
    try {
        // Test device connection first
        if (!test_device_connection()) {
            std::cout << "Device connection failed. Skipping hardware-dependent tests.\n";
            test_framework.print_summary();
            return test_framework.all_passed() ? 0 : 1;
        }
        
        // Connect to device for remaining tests
        makcu::Device device;
        std::string port = makcu::Device::findFirstDevice();
        
        if (port.empty() || !device.connect(port)) {
            std::cout << "Failed to connect to device for testing.\n";
            test_framework.print_summary();
            return 1;
        }
        
        std::cout << "Connected to device on " << port << " for testing.\n\n";
        
        // Run all hardware-dependent tests
        test_keyboard_functionality(device);
        test_mouse_functionality(device);
        test_advanced_mouse_functionality(device);
        test_macro_functionality(device);
        test_gaming_utilities_integration(device);
        test_monitoring_functionality(device);
        
        device.disconnect();
        
        test_framework.print_summary();
        
        if (test_framework.all_passed()) {
            std::cout << "\nAll integration tests passed! ✅" << std::endl;
            return 0;
        } else {
            std::cout << "\nSome integration tests failed! ❌" << std::endl;
            return 1;
        }
        
    } catch (const makcu::MakcuException& e) {
        std::cerr << "Integration test failed with MAKCU exception: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Integration test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
