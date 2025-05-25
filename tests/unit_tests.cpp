#include "../makcu-cpp/include/makcu.h"
#include "../makcu-cpp/include/utilities.h"
#include <iostream>
#include <cassert>
#include <sstream>

// Simple test framework
class TestFramework {
private:
    int tests_run = 0;
    int tests_passed = 0;

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

    void assert_equal(const std::string& expected, const std::string& actual, const std::string& test_name) {
        assert_true(expected == actual, test_name);
    }

    void print_summary() {
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Tests run: " << tests_run << std::endl;
        std::cout << "Tests passed: " << tests_passed << std::endl;
        std::cout << "Tests failed: " << (tests_run - tests_passed) << std::endl;
        std::cout << "Success rate: " << (tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0) << "%" << std::endl;
    }
};

TestFramework test_framework;

void test_key_code_utilities() {
    std::cout << "\n--- Testing Key Code Utilities ---" << std::endl;
    
    // Test key code to string conversion
    test_framework.assert_equal("A", makcu::keyCodeToString(makcu::KeyCode::KEY_A), "KeyCode::KEY_A to string");
    test_framework.assert_equal("SPACE", makcu::keyCodeToString(makcu::KeyCode::KEY_SPACEBAR), "KeyCode::KEY_SPACEBAR to string");
    test_framework.assert_equal("ENTER", makcu::keyCodeToString(makcu::KeyCode::KEY_ENTER), "KeyCode::KEY_ENTER to string");
    
    // Test string to key code conversion
    test_framework.assert_true(makcu::stringToKeyCode("A") == makcu::KeyCode::KEY_A, "String 'A' to KeyCode");
    test_framework.assert_true(makcu::stringToKeyCode("SPACE") == makcu::KeyCode::KEY_SPACEBAR, "String 'SPACE' to KeyCode");
    test_framework.assert_true(makcu::stringToKeyCode("ENTER") == makcu::KeyCode::KEY_ENTER, "String 'ENTER' to KeyCode");
    
    // Test case insensitivity
    test_framework.assert_true(makcu::stringToKeyCode("a") == makcu::KeyCode::KEY_A, "String 'a' (lowercase) to KeyCode");
    test_framework.assert_true(makcu::stringToKeyCode("space") == makcu::KeyCode::KEY_SPACEBAR, "String 'space' (lowercase) to KeyCode");
}

void test_mouse_button_utilities() {
    std::cout << "\n--- Testing Mouse Button Utilities ---" << std::endl;
    
    // Test mouse button to string conversion
    test_framework.assert_equal("LEFT", makcu::mouseButtonToString(makcu::MouseButton::LEFT), "MouseButton::LEFT to string");
    test_framework.assert_equal("RIGHT", makcu::mouseButtonToString(makcu::MouseButton::RIGHT), "MouseButton::RIGHT to string");
    test_framework.assert_equal("MIDDLE", makcu::mouseButtonToString(makcu::MouseButton::MIDDLE), "MouseButton::MIDDLE to string");
    test_framework.assert_equal("SIDE4", makcu::mouseButtonToString(makcu::MouseButton::SIDE4), "MouseButton::SIDE4 to string");
    test_framework.assert_equal("SIDE5", makcu::mouseButtonToString(makcu::MouseButton::SIDE5), "MouseButton::SIDE5 to string");
}

void test_mouse_button_states() {
    std::cout << "\n--- Testing Mouse Button States ---" << std::endl;
    
    makcu::MouseButtonStates states;
    
    // Test initial state (all false)
    test_framework.assert_true(!states.left, "Initial left button state is false");
    test_framework.assert_true(!states.right, "Initial right button state is false");
    test_framework.assert_true(!states.middle, "Initial middle button state is false");
    test_framework.assert_true(!states.side4, "Initial side4 button state is false");
    test_framework.assert_true(!states.side5, "Initial side5 button state is false");
    
    // Test operator[]
    test_framework.assert_true(!states[makcu::MouseButton::LEFT], "operator[] for LEFT button (initial state)");
    test_framework.assert_true(!states[makcu::MouseButton::RIGHT], "operator[] for RIGHT button (initial state)");
    
    // Test set method
    states.set(makcu::MouseButton::LEFT, true);
    test_framework.assert_true(states.left, "Set left button to true");
    test_framework.assert_true(states[makcu::MouseButton::LEFT], "operator[] for LEFT button (after set to true)");
    
    states.set(makcu::MouseButton::RIGHT, true);
    test_framework.assert_true(states.right, "Set right button to true");
    
    states.set(makcu::MouseButton::LEFT, false);
    test_framework.assert_true(!states.left, "Set left button back to false");
    test_framework.assert_true(states[makcu::MouseButton::RIGHT], "Right button still true after changing left");
}

void test_device_info() {
    std::cout << "\n--- Testing Device Info ---" << std::endl;
    
    makcu::DeviceInfo info;
    info.port = "COM3";
    info.description = "Test Device";
    info.vid = 0x1234;
    info.pid = 0x5678;
    info.isConnected = true;
    
    test_framework.assert_equal("COM3", info.port, "DeviceInfo port assignment");
    test_framework.assert_equal("Test Device", info.description, "DeviceInfo description assignment");
    test_framework.assert_true(info.vid == 0x1234, "DeviceInfo VID assignment");
    test_framework.assert_true(info.pid == 0x5678, "DeviceInfo PID assignment");
    test_framework.assert_true(info.isConnected, "DeviceInfo isConnected assignment");
}

void test_serial_port_utilities() {
    std::cout << "\n--- Testing Serial Port Utilities ---" << std::endl;
    
    // Test port enumeration (doesn't require device)
    auto ports = makcu::SerialPort::getAvailablePorts();
    test_framework.assert_true(true, "Serial port enumeration completed without crash");
    
    auto makcuPorts = makcu::SerialPort::findMakcuPorts();
    test_framework.assert_true(true, "MAKCU port search completed without crash");
    
    std::cout << "Found " << ports.size() << " serial ports total" << std::endl;
    std::cout << "Found " << makcuPorts.size() << " MAKCU ports" << std::endl;
}

void test_device_discovery() {
    std::cout << "\n--- Testing Device Discovery ---" << std::endl;
    
    // Test device discovery (doesn't require device connection)
    auto devices = makcu::Device::findDevices();
    test_framework.assert_true(true, "Device discovery completed without crash");
    
    std::string firstDevice = makcu::Device::findFirstDevice();
    test_framework.assert_true(true, "Find first device completed without crash");
    
    std::cout << "Found " << devices.size() << " MAKCU devices" << std::endl;
    if (!firstDevice.empty()) {
        std::cout << "First device on port: " << firstDevice << std::endl;
    }
}

void test_macro_recorder() {
    std::cout << "\n--- Testing Macro Recorder ---" << std::endl;
    
    makcu::MacroRecorder recorder;
    
    // Test initial state
    test_framework.assert_true(!recorder.isRecording(), "Recorder not recording initially");
    test_framework.assert_true(recorder.getActionCount() == 0, "Recorder has no actions initially");
    test_framework.assert_true(recorder.getTotalDuration().count() == 0, "Recorder has zero duration initially");
    
    // Test adding actions manually
    recorder.addKeyPress(makcu::KeyCode::KEY_A);
    test_framework.assert_true(recorder.getActionCount() == 1, "Action count increases after adding key press");
    
    recorder.addMouseClick(makcu::MouseButton::LEFT);
    test_framework.assert_true(recorder.getActionCount() == 2, "Action count increases after adding mouse click");
    
    recorder.addDelay(100);
    test_framework.assert_true(recorder.getActionCount() == 3, "Action count increases after adding delay");
    
    // Test clearing
    recorder.clear();
    test_framework.assert_true(recorder.getActionCount() == 0, "Action count resets after clear");
    
    // Test settings
    recorder.setRecordMouseMovement(true);
    recorder.setMinimumDelay(50);
    recorder.setTimestampMode(false);
    test_framework.assert_true(true, "Recorder settings can be changed without error");
}

void test_gaming_utilities() {
    std::cout << "\n--- Testing Gaming Utilities ---" << std::endl;
    
    // Test humanization functions
    uint32_t baseDelay = 100;
    uint32_t humanizedDelay = makcu::GamingUtilities::humanizeDelay(baseDelay, 20);
    test_framework.assert_true(humanizedDelay >= 80 && humanizedDelay <= 120, "Humanized delay within expected range");
    
    auto humanizedMovement = makcu::GamingUtilities::humanizeMovement(100, 50, 5);
    test_framework.assert_true(humanizedMovement.first >= 95 && humanizedMovement.first <= 105, "Humanized X movement within range");
    test_framework.assert_true(humanizedMovement.second >= 45 && humanizedMovement.second <= 55, "Humanized Y movement within range");
    
    // Test multiple calls produce different results
    uint32_t delay1 = makcu::GamingUtilities::humanizeDelay(100, 20);
    uint32_t delay2 = makcu::GamingUtilities::humanizeDelay(100, 20);
    uint32_t delay3 = makcu::GamingUtilities::humanizeDelay(100, 20);
    
    // Very unlikely that all three would be exactly the same
    test_framework.assert_true(!(delay1 == delay2 && delay2 == delay3), "Humanized delays show variance");
}

void test_action_serialization() {
    std::cout << "\n--- Testing Action Serialization ---" << std::endl;
    
    // Test KeyAction serialization
    makcu::KeyAction keyAction(makcu::ActionType::KEY_PRESS, makcu::KeyCode::KEY_A, 100);
    std::string serialized = keyAction.serialize();
    test_framework.assert_true(!serialized.empty(), "KeyAction serialization produces non-empty string");
    test_framework.assert_true(serialized.find("2") != std::string::npos, "KeyAction serialization contains action type");
    
    // Test TypeStringAction serialization
    makcu::TypeStringAction typeAction("Hello World");
    std::string typeSerialized = typeAction.serialize();
    test_framework.assert_true(!typeSerialized.empty(), "TypeStringAction serialization produces non-empty string");
    test_framework.assert_true(typeSerialized.find("Hello World") != std::string::npos, "TypeStringAction serialization contains text");
    
    // Test MouseButtonAction serialization
    makcu::MouseButtonAction mouseAction(makcu::ActionType::MOUSE_CLICK, makcu::MouseButton::LEFT, 2);
    std::string mouseSerialized = mouseAction.serialize();
    test_framework.assert_true(!mouseSerialized.empty(), "MouseButtonAction serialization produces non-empty string");
    
    // Test DelayAction serialization
    makcu::DelayAction delayAction(500);
    std::string delaySerialized = delayAction.serialize();
    test_framework.assert_true(!delaySerialized.empty(), "DelayAction serialization produces non-empty string");
    test_framework.assert_true(delaySerialized.find("500") != std::string::npos, "DelayAction serialization contains delay value");
}

void test_exception_handling() {
    std::cout << "\n--- Testing Exception Handling ---" << std::endl;
    
    // Test exception creation and what() method
    makcu::MakcuException baseException("Base error message");
    test_framework.assert_equal("Base error message", std::string(baseException.what()), "MakcuException what() method");
    
    makcu::ConnectionException connectionException("Connection failed");
    test_framework.assert_true(std::string(connectionException.what()).find("Connection error") != std::string::npos, 
                              "ConnectionException includes prefix");
    test_framework.assert_true(std::string(connectionException.what()).find("Connection failed") != std::string::npos, 
                              "ConnectionException includes original message");
    
    makcu::CommandException commandException("Invalid command");
    test_framework.assert_true(std::string(commandException.what()).find("Command error") != std::string::npos, 
                              "CommandException includes prefix");
    test_framework.assert_true(std::string(commandException.what()).find("Invalid command") != std::string::npos, 
                              "CommandException includes original message");
}

int main() {
    std::cout << "MAKCU C++ Library Unit Tests\n";
    std::cout << "=============================\n";
    
    try {
        test_key_code_utilities();
        test_mouse_button_utilities();
        test_mouse_button_states();
        test_device_info();
        test_serial_port_utilities();
        test_device_discovery();
        test_macro_recorder();
        test_gaming_utilities();
        test_action_serialization();
        test_exception_handling();
        
        test_framework.print_summary();
        
        if (test_framework.tests_passed == test_framework.tests_run) {
            std::cout << "\nAll tests passed! ✅" << std::endl;
            return 0;
        } else {
            std::cout << "\nSome tests failed! ❌" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Test execution failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
