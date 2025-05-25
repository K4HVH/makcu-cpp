#define _USE_MATH_DEFINES
#include "../makcu-cpp/include/makcu.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

// Mouse automation example
// This example demonstrates various mouse automation capabilities

void waitForUser(const std::string& message) {
    std::cout << message << " Press Enter to continue...";
    std::cin.get();
}

void demonstrateBasicMovement(makcu::Device& device) {
    std::cout << "\n--- Basic Mouse Movement Demo ---\n";
    waitForUser("This will move the mouse cursor in different patterns.");
    
    std::cout << "Moving mouse in a square pattern...\n";
    
    // Move in a square
    device.mouseMove(200, 0);    // Right
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    device.mouseMove(0, 200);    // Down
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    device.mouseMove(-200, 0);   // Left
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    device.mouseMove(0, -200);   // Up
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void demonstrateCircularMovement(makcu::Device& device) {
    std::cout << "\n--- Circular Movement Demo ---\n";
    waitForUser("This will move the mouse in a circle.");
    
    std::cout << "Moving mouse in a circle...\n";
    
    const int radius = 100;
    const int steps = 36; // 10-degree increments
    
    for (int i = 0; i < steps; ++i) {
        double angle = (i * 2 * M_PI) / steps;
        int x = static_cast<int>(radius * cos(angle));
        int y = static_cast<int>(radius * sin(angle));
        
        // Calculate relative movement from previous position
        static int prevX = 0, prevY = 0;
        int deltaX = x - prevX;
        int deltaY = y - prevY;
        
        device.mouseMove(deltaX, deltaY);
        prevX = x;
        prevY = y;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Return to center
    device.mouseMove(-radius, 0);
}

void demonstrateMouseClicks(makcu::Device& device) {
    std::cout << "\n--- Mouse Clicks Demo ---\n";
    waitForUser("This will demonstrate different types of mouse clicks.");
    
    std::cout << "Single left click...\n";
    device.mouseClick(makcu::MouseButton::LEFT);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "Double left click...\n";
    device.mouseClick(makcu::MouseButton::LEFT, 2);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "Right click...\n";
    device.mouseClick(makcu::MouseButton::RIGHT);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Move away from any context menu that might have appeared
    device.mouseMove(100, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    device.mouseClick(makcu::MouseButton::LEFT); // Click to dismiss
    
    std::cout << "Middle click...\n";
    device.mouseClick(makcu::MouseButton::MIDDLE);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void demonstrateMouseDragDrop(makcu::Device& device) {
    std::cout << "\n--- Drag and Drop Demo ---\n";
    waitForUser("This will demonstrate drag and drop operation.");
    
    std::cout << "Performing drag and drop...\n";
    
    // Press and hold left button
    device.mouseDown(makcu::MouseButton::LEFT);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Drag to new position
    device.mouseMove(150, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Release the button
    device.mouseUp(makcu::MouseButton::LEFT);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void demonstrateScrolling(makcu::Device& device) {
    std::cout << "\n--- Mouse Scrolling Demo ---\n";
    waitForUser("This will demonstrate mouse wheel scrolling.");
    
    std::cout << "Scrolling up...\n";
    for (int i = 0; i < 5; ++i) {
        device.mouseWheel(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "Scrolling down...\n";
    for (int i = 0; i < 5; ++i) {
        device.mouseWheel(-1);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void demonstratePreciseMovement(makcu::Device& device) {
    std::cout << "\n--- Precise Movement Demo ---\n";
    waitForUser("This will demonstrate precise mouse positioning (if screen bounds are set).");
    
    // Set screen bounds (assuming 1920x1080 screen)
    device.mouseSetScreenBounds(1920, 1080);
    
    // Calibrate mouse position
    std::cout << "Calibrating mouse position...\n";
    device.mouseCalibrate();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    std::cout << "Moving to specific coordinates...\n";
    
    // Move to different screen positions
    std::vector<std::pair<int, int>> positions = {
        {960, 540},  // Center
        {100, 100},  // Top-left
        {1820, 100}, // Top-right  
        {1820, 980}, // Bottom-right
        {100, 980},  // Bottom-left
        {960, 540}   // Back to center
    };
    
    for (const auto& pos : positions) {
        std::cout << "Moving to (" << pos.first << ", " << pos.second << ")...\n";
        device.mouseMoveTo(pos.first, pos.second);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void demonstrateComplexPattern(makcu::Device& device) {
    std::cout << "\n--- Complex Pattern Demo ---\n";
    waitForUser("This will draw a spiral pattern.");
    
    std::cout << "Drawing spiral pattern...\n";
    
    const int maxRadius = 150;
    const int steps = 100;
    
    device.mouseDown(makcu::MouseButton::LEFT); // Start "drawing"
    
    for (int i = 0; i < steps; ++i) {
        double angle = (i * 4 * M_PI) / steps; // 2 full rotations
        double radius = (maxRadius * i) / steps; // Expanding radius
        
        int x = static_cast<int>(radius * cos(angle));
        int y = static_cast<int>(radius * sin(angle));
        
        // Calculate relative movement
        static int prevX = 0, prevY = 0;
        int deltaX = x - prevX;
        int deltaY = y - prevY;
        
        device.mouseMove(deltaX, deltaY);
        prevX = x;
        prevY = y;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    device.mouseUp(makcu::MouseButton::LEFT); // Stop "drawing"
}

int main() {
    std::cout << "MAKCU Mouse Automation Example\n";
    std::cout << "==============================\n\n";
    std::cout << "This example demonstrates various mouse automation capabilities.\n";
    std::cout << "You can open a drawing program to see the effects better.\n\n";

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
        waitForUser("Position your mouse cursor where you want to start the demo.");

        // Run demonstrations
        demonstrateBasicMovement(device);
        demonstrateCircularMovement(device);
        demonstrateMouseClicks(device);
        demonstrateMouseDragDrop(device);
        demonstrateScrolling(device);
        demonstratePreciseMovement(device);
        demonstrateComplexPattern(device);

        std::cout << "\nAll mouse demonstrations completed!\n";
        device.disconnect();
        std::cout << "Disconnected from device.\n";

    } catch (const makcu::MakcuException& e) {
        std::cerr << "MAKCU Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
