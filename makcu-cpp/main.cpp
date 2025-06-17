#include "include/makcu.h"
#include "include/serialport.h"
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

    // Test 4: Async operations
    std::cout << "4. Testing async operations...\n";
    auto async_start = std::chrono::high_resolution_clock::now();

    std::vector<std::future<bool>> futures;
    futures.push_back(device.mouseMoveAsync(25, 25));
    futures.push_back(device.clickAsync(makcu::MouseButton::LEFT));
    futures.push_back(device.mouseMoveSmoothAsync(-25, -25, 10));
    futures.push_back(device.mouseWheelAsync(2));
    futures.push_back(device.mouseMoveBezierAsync(15, 15, 8, 7, 7));

    // Wait for all to complete
    for (auto& future : futures) {
        future.get();
    }

    auto async_end = std::chrono::high_resolution_clock::now();
    auto async_ms = std::chrono::duration_cast<std::chrono::milliseconds>(async_end - async_start).count();
    std::cout << "   5 async operations: " << async_ms << "ms\n";

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

        // Demonstrate async operations
        std::cout << "Performing async operations...\n";

        auto versionFuture = device.getVersionAsync();
        auto serialFuture = device.getMouseSerialAsync();

        // Wait for results
        std::cout << "Firmware version: " << versionFuture.get() << "\n";
        std::cout << "Mouse serial: " << serialFuture.get() << "\n";

        // Async disconnect
        auto disconnectFuture = device.disconnectAsync();
        disconnectFuture.wait();
        std::cout << "Disconnected asynchronously\n";
    }
    else {
        std::cout << "Failed to connect\n";
    }
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