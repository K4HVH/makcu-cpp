#include "include/makcu.h"
#include "include/serialport.h"
#include "include/human_patterns.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <future>

void mouseButtonCallback(makcu::MouseButton button, bool isPressed) {
    std::string buttonName = makcu::mouseButtonToString(button);
    std::string state = isPressed ? "PRESSED" : "RELEASED";
    std::cout << "Button " << buttonName << " " << state << std::endl;
}

void connectionCallback(bool connected) {
    std::cout << "Device " << (connected ? "CONNECTED" : "DISCONNECTED") << std::endl;
}

void performanceTest() {
    std::cout << "\n=== PERFORMANCE TEST ===\n";

    makcu::Device device;
    if (!device.connect()) {
        std::cout << "Failed to connect for performance test\n";
        return;
    }

    // Enable high-performance mode and profiling
    device.enableHighPerformanceMode(true);
    makcu::PerformanceProfiler::enableProfiling(true);

    auto start = std::chrono::high_resolution_clock::now();

    // Test 1: Rapid fire mouse movements (gaming scenario)
    std::cout << "1. Testing rapid mouse movements (100 commands)...\n";
    for (int i = 0; i < 100; ++i) {
        device.mouseMove(i % 10, i % 10);
    }

    auto movements_end = std::chrono::high_resolution_clock::now();
    auto movements_ms = std::chrono::duration_cast<std::chrono::milliseconds>(movements_end - start).count();
    std::cout << "   100 movements: " << movements_ms << "ms (" << movements_ms / 100.0 << "ms avg)\n";

    // Test 2: Rapid clicking sequence
    std::cout << "2. Testing rapid clicking (50 clicks)...\n";
    auto click_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 50; ++i) {
        device.click(makcu::MouseButton::LEFT);
    }
    auto click_end = std::chrono::high_resolution_clock::now();
    auto click_ms = std::chrono::duration_cast<std::chrono::milliseconds>(click_end - click_start).count();
    std::cout << "   50 clicks: " << click_ms << "ms (" << click_ms / 50.0 << "ms avg)\n";

    // Test 3: Batch operations
    std::cout << "3. Testing batch operations...\n";
    auto batch_start = std::chrono::high_resolution_clock::now();

    auto batch = device.createBatch();
    batch.move(50, 0)
        .click(makcu::MouseButton::LEFT)
        .move(0, 50)
        .click(makcu::MouseButton::RIGHT)
        .move(-50, 0)
        .click(makcu::MouseButton::MIDDLE)
        .move(0, -50)
        .scroll(3)
        .scroll(-3);
    batch.execute();

    auto batch_end = std::chrono::high_resolution_clock::now();
    auto batch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(batch_end - batch_start).count();
    std::cout << "   Batch (9 commands): " << batch_ms << "ms\n";

    // Test 4: High-performance operations
    std::cout << "4. Testing high-performance synchronous operations...\n";
    auto async_start = std::chrono::high_resolution_clock::now();

    // Use synchronous methods for maximum performance (fire-and-forget)
    device.mouseMove(25, 25);
    device.click(makcu::MouseButton::LEFT);
    device.mouseMoveSmooth(-25, -25, 10);
    device.mouseWheel(2);
    device.mouseMoveBezier(15, 15, 8, 7, 7);

    auto async_end = std::chrono::high_resolution_clock::now();
    auto async_ms = std::chrono::duration_cast<std::chrono::milliseconds>(async_end - async_start).count();
    std::cout << "   5 synchronous operations: " << async_ms << "ms\n";

    auto total_end = std::chrono::high_resolution_clock::now();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(total_end - start).count();
    std::cout << "\nTotal test time: " << total_ms << "ms\n";

    // Show performance statistics
    std::cout << "\n=== PERFORMANCE STATISTICS ===\n";
    auto stats = makcu::PerformanceProfiler::getStats();
    for (const auto& [command, data] : stats) {
        auto [count, total_us] = data;
        if (count > 0) {
            double avg_us = static_cast<double>(total_us) / count;
            std::cout << command << ": " << count << " calls, avg " << avg_us << "us\n";
        }
    }

    device.disconnect();
}

void gamingScenarioDemo() {
    std::cout << "\n=== GAMING SCENARIO DEMO ===\n";

    makcu::Device device;
    if (!device.connect()) {
        std::cout << "Failed to connect for gaming demo\n";
        return;
    }

    device.enableHighPerformanceMode(true);

    std::cout << "Simulating FPS gaming scenario...\n";

    // Simulate recoil control with smooth movement
    std::cout << "1. Recoil control pattern...\n";
    for (int i = 0; i < 10; ++i) {
        device.mouseMoveSmooth(0, -2, 3);  // Small downward adjustments
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Simulate rapid fire with perfect timing
    std::cout << "2. Rapid fire sequence...\n";
    for (int i = 0; i < 20; ++i) {
        device.click(makcu::MouseButton::LEFT);
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 20 CPS
    }

    // Simulate weapon switch + aim
    std::cout << "3. Weapon switch + aim sequence...\n";
    device.mouseWheel(1);  // Switch weapon
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    device.mouseMoveSmooth(75, -25, 15);  // Smooth aim to target
    device.click(makcu::MouseButton::RIGHT);  // Aim down sights
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    device.click(makcu::MouseButton::LEFT);   // Fire
    device.click(makcu::MouseButton::RIGHT);  // Release ADS

    std::cout << "Gaming scenario complete!\n";
    device.disconnect();
}

void lockingDemo() {
    std::cout << "\n=== ADVANCED LOCKING DEMO ===\n";

    makcu::Device device;
    if (!device.connect()) {
        std::cout << "Failed to connect for locking demo\n";
        return;
    }

    // Demonstrate input masking
    std::cout << "Testing input locking capabilities...\n";

    // Lock X-axis movement
    std::cout << "1. Locking X-axis for 3 seconds...\n";
    device.lockMouseX(true);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    device.lockMouseX(false);
    std::cout << "   X-axis unlocked\n";

    // Lock left mouse button
    std::cout << "2. Locking left mouse button for 2 seconds...\n";
    device.lockMouseLeft(true);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    device.lockMouseLeft(false);
    std::cout << "   Left button unlocked\n";

    // Show all lock states
    std::cout << "3. Current lock states:\n";
    auto lockStates = device.getAllLockStates();
    for (const auto& [name, locked] : lockStates) {
        std::cout << "   " << name << ": " << (locked ? "LOCKED" : "UNLOCKED") << "\n";
    }

    device.disconnect();
}

void asyncDemo() {
    std::cout << "\n=== ASYNC OPERATIONS DEMO ===\n";

    // Demonstrate async connection
    makcu::Device device;

    std::cout << "Connecting asynchronously...\n";
    auto connectFuture = device.connectAsync();

    // Do other work while connecting
    std::cout << "Doing other work while connecting...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (connectFuture.get()) {
        std::cout << "Connected successfully!\n";

        // Demonstrate synchronous operations
        std::cout << "Performing device information queries...\n";

        // Use synchronous methods
        std::string version = device.getVersion();
        std::string serial = device.getMouseSerial();

        std::cout << "Firmware version: " << version << "\n";
        std::cout << "Mouse serial: " << serial << "\n";

        // Synchronous disconnect
        device.disconnect();
        std::cout << "Disconnected\n";
    }
    else {
        std::cout << "Failed to connect\n";
    }
}

void humanLikePatternsDemo() {
    std::cout << "\n=== ADVANCED HUMAN-LIKE PATTERNS DEMO ===\n";

    makcu::Device device;
    if (!device.connect()) {
        std::cout << "Failed to connect for human patterns demo\n";
        return;
    }

    auto* patterns = device.getHumanPatterns();
    if (!patterns) {
        std::cout << "Failed to get human patterns interface\n";
        device.disconnect();
        return;
    }

    std::cout << "Testing advanced human-like interaction patterns...\n";

    // Test 1: Personality-based profiles
    std::cout << "1. Testing personality-based profiles...\n";
    
    std::cout << "   PRECISION profile (focused, minimal jitter)...\n";
    patterns->clickHumanLike(makcu::MouseButton::LEFT, 3, makcu::HumanPatterns::Profile::PRECISION);
    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    std::cout << "   CAFFEINATED profile (fast, jittery, overshoot movement)...\n";
    patterns->clickHumanLike(makcu::MouseButton::LEFT, 5, makcu::HumanPatterns::Profile::CAFFEINATED);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "   SLEEPY profile (slow, high variation, fatigue enabled)...\n";
    patterns->clickHumanLike(makcu::MouseButton::LEFT, 2, makcu::HumanPatterns::Profile::SLEEPY);
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    // Test 2: Advanced movement curves
    std::cout << "2. Testing advanced movement curves...\n";
    
    std::cout << "   Bezier natural curve movement...\n";
    patterns->moveHumanLike(100, 50, makcu::HumanPatterns::Profile::CASUAL);
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    std::cout << "   Ease-in-out curve movement...\n";
    patterns->moveHumanLike(-50, -25, makcu::HumanPatterns::Profile::PRECISION);
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    std::cout << "   Overshoot curve movement (caffeinated style)...\n";
    patterns->moveHumanLike(25, 75, makcu::HumanPatterns::Profile::CAFFEINATED);
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    // Test 3: Human-like drag operations
    std::cout << "3. Testing human-like drag operations...\n";
    
    std::cout << "   Casual drag with natural movement...\n";
    patterns->dragHumanLike(0, 0, 80, 40, makcu::MouseButton::LEFT, makcu::HumanPatterns::Profile::CASUAL);
    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    std::cout << "   Gaming drag (linear, fast)...\n";
    patterns->dragHumanLike(80, 40, 0, 0, makcu::MouseButton::LEFT, makcu::HumanPatterns::Profile::GAMING);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Test 4: Human-like scrolling
    std::cout << "4. Testing human-like scrolling...\n";
    
    std::cout << "   Casual scroll (3 steps)...\n";
    patterns->scrollHumanLike(5, 3, makcu::HumanPatterns::Profile::CASUAL);
    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    // Test 5: Fatigue simulation
    std::cout << "5. Testing fatigue simulation...\n";
    
    std::cout << "   Simulating session fatigue (sleepy profile)...\n";
    patterns->setSessionFatigue(0.3);
    for (int i = 0; i < 10; ++i) {
        patterns->clickHumanLike(makcu::MouseButton::LEFT, 1, makcu::HumanPatterns::Profile::SLEEPY);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    patterns->setSessionFatigue(0.0);

    // Test 6: Session fatigue simulation
    std::cout << "6. Testing session fatigue simulation...\n";
    
    std::cout << "   Normal clicks...\n";
    patterns->clickHumanLike(makcu::MouseButton::LEFT, 3, makcu::HumanPatterns::Profile::CASUAL);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "   Setting fatigue level to 50%...\n";
    patterns->setSessionFatigue(0.5);
    patterns->clickHumanLike(makcu::MouseButton::LEFT, 3, makcu::HumanPatterns::Profile::SLEEPY);
    patterns->setSessionFatigue(0.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    // Test 7: Mixed performance scenarios
    std::cout << "7. Testing mixed performance scenarios...\n";
    
    std::cout << "   Raw performance move (instant)...\n";
    device.mouseMove(50, 0);
    
    std::cout << "   Human-like click (with built-in jitter)...\n";
    patterns->clickHumanLike(makcu::MouseButton::LEFT, 1, makcu::HumanPatterns::Profile::CASUAL);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    std::cout << "   Raw performance sequence...\n";
    device.click(makcu::MouseButton::RIGHT);
    device.mouseWheel(2);
    
    std::cout << "   Back to human-like with sleepy profile...\n";
    patterns->clickHumanLike(makcu::MouseButton::MIDDLE, 1, makcu::HumanPatterns::Profile::SLEEPY);

    // Test 8: Custom profile creation
    std::cout << "8. Testing custom profile creation...\n";
    
    makcu::HumanPatterns::CustomProfile customProfile = {
        std::chrono::milliseconds(80), std::chrono::milliseconds(140),    // press timing
        std::chrono::milliseconds(90), std::chrono::milliseconds(160),    // release timing
        10, 0.7,                                                          // jitter: 10px radius, 70% chance
        std::chrono::milliseconds(5), std::chrono::milliseconds(20),      // jitter delays
        makcu::HumanPatterns::MovementCurve::EASE_IN_OUT, 12, 0.9,       // movement
        makcu::HumanPatterns::ClickVariation::MODERATE_VARIATION, 0.12,   // click variation
        true, 0.15, 40,                                                   // fatigue simulation
        true, 1.2, 5                                                      // acceleration
    };
    
    patterns->setCustomProfile("expert_gamer", customProfile);
    patterns->setActiveProfile("expert_gamer");
    
    std::cout << "   Created 'expert_gamer' custom profile with acceleration and moderate jitter\n";
    std::cout << "   Testing expert gamer custom profile...\n";
    
    for (int i = 0; i < 8; ++i) {
        patterns->clickHumanLike(makcu::MouseButton::LEFT, 1, makcu::HumanPatterns::Profile::CASUAL);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Test 9: Profile comparison
    std::cout << "9. Demonstrating profile timing differences...\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    patterns->resetToDefaultProfile(makcu::HumanPatterns::Profile::GAMING);
    patterns->clickHumanLike(makcu::MouseButton::LEFT, 1, makcu::HumanPatterns::Profile::GAMING);
    auto gaming_end = std::chrono::high_resolution_clock::now();
    auto gaming_ms = std::chrono::duration_cast<std::chrono::milliseconds>(gaming_end - start).count();
    std::cout << "   Gaming profile click: " << gaming_ms << "ms\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    start = std::chrono::high_resolution_clock::now();
    patterns->clickHumanLike(makcu::MouseButton::LEFT, 1, makcu::HumanPatterns::Profile::SLEEPY);
    auto sleepy_end = std::chrono::high_resolution_clock::now();
    auto sleepy_ms = std::chrono::duration_cast<std::chrono::milliseconds>(sleepy_end - start).count();
    std::cout << "   Sleepy profile click: " << sleepy_ms << "ms\n";

    std::cout << "\nAdvanced human-like patterns demo completed!\n";
    
    device.disconnect();
}

int main() {
    std::cout << "MAKCU C++ High-Performance Library Demo\n";
    std::cout << "=======================================\n\n";

    try {
        // Find devices
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

        // Basic connectivity test
        std::cout << "\n=== BASIC CONNECTIVITY TEST ===\n";
        makcu::Device device;

        // Set up callbacks
        device.setMouseButtonCallback(mouseButtonCallback);
        device.setConnectionCallback(connectionCallback);

        std::cout << "Connecting to " << devices[0].port << "...\n";
        if (!device.connect(devices[0].port)) {
            std::cout << "Failed to connect to device.\n";
            return 1;
        }

        auto deviceInfo = device.getDeviceInfo();
        std::cout << "Device Info:\n";
        std::cout << "  Port: " << deviceInfo.port << "\n";
        std::cout << "  VID: 0x" << std::hex << deviceInfo.vid << "\n";
        std::cout << "  PID: 0x" << std::hex << deviceInfo.pid << std::dec << "\n";
        std::cout << "  Version: " << device.getVersion() << "\n";

        // Basic functionality test
        std::cout << "\n=== BASIC FUNCTIONALITY TEST ===\n";
        std::cout << "Testing basic mouse operations...\n";

        device.mouseMove(50, 0);
        device.mouseMove(-50, 0);
        device.click(makcu::MouseButton::LEFT);
        device.mouseWheel(3);
        device.mouseWheel(-3);

        std::cout << "Basic test completed successfully!\n";
        device.disconnect();

        // Run performance tests
        performanceTest();

        // Run gaming scenario
        gamingScenarioDemo();

        // Run locking demo
        lockingDemo();

        // Run async demo
        asyncDemo();

        // Run human-like patterns demo
        humanLikePatternsDemo();

        std::cout << "\n=== ALL DEMOS COMPLETED ===\n";

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