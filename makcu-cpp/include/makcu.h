#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <cstdint>
#include <exception>

namespace makcu {

    // Forward declaration
    class SerialPort;

    // Enums must be defined before use
    enum class MouseButton : uint8_t {
        LEFT = 0,
        RIGHT = 1,
        MIDDLE = 2,
        SIDE4 = 3,
        SIDE5 = 4
    };

    // Note: MAKCU device appears to be mouse-only based on testing
    // Keyboard functionality is not supported on this device
    enum class ConnectionStatus {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        CONNECTION_ERROR,
    };

    // Simple structs
    struct DeviceInfo {
        std::string port;
        std::string description;
        uint16_t vid;
        uint16_t pid;
        bool isConnected;
    };

    struct MouseButtonStates {
        bool left;
        bool right;
        bool middle;
        bool side4;
        bool side5;

        MouseButtonStates() : left(false), right(false), middle(false), side4(false), side5(false) {}

        bool operator[](MouseButton button) const {
            switch (button) {
            case MouseButton::LEFT: return left;
            case MouseButton::RIGHT: return right;
            case MouseButton::MIDDLE: return middle;
            case MouseButton::SIDE4: return side4;
            case MouseButton::SIDE5: return side5;
            }
            return false;
        }

        void set(MouseButton button, bool state) {
            switch (button) {
            case MouseButton::LEFT: left = state; break;
            case MouseButton::RIGHT: right = state; break;
            case MouseButton::MIDDLE: middle = state; break;
            case MouseButton::SIDE4: side4 = state; break;
            case MouseButton::SIDE5: side5 = state; break;
            }
        }
    };

    // Exception classes
    class MakcuException : public std::exception {
    public:
        explicit MakcuException(const std::string& message) : m_message(message) {}
        const char* what() const noexcept override { return m_message.c_str(); }
    private:
        std::string m_message;
    };

    class ConnectionException : public MakcuException {
    public:
        explicit ConnectionException(const std::string& message)
            : MakcuException("Connection error: " + message) {
        }
    };

    class CommandException : public MakcuException {
    public:
        explicit CommandException(const std::string& message)
            : MakcuException("Command error: " + message) {
        }
    };

    // Main Device class - MAKCU Mouse Controller
    class Device {
    public:
        // Callback types
        typedef std::function<void(MouseButton, bool)> MouseButtonCallback;

        // Constructor and destructor
        Device();
        ~Device();

        // Static methods
        static std::vector<DeviceInfo> findDevices();
        static std::string findFirstDevice();

        // Connection
        bool connect(const std::string& port = "");
        void disconnect();
        bool isConnected() const;
        ConnectionStatus getStatus() const;

        // Device info
        DeviceInfo getDeviceInfo() const;
        std::string getVersion() const;

        // Mouse button control
        bool mouseDown(MouseButton button);
        bool mouseUp(MouseButton button);
        bool mouseButtonState(MouseButton button); // Get current state

        // Mouse movement (enhanced v3.2 support)
        bool mouseMove(int32_t x, int32_t y);                                    // Instant movement
        bool mouseMoveSmooth(int32_t x, int32_t y, uint32_t segments);          // Randomized curve
        bool mouseMoveBezier(int32_t x, int32_t y, uint32_t segments,
            int32_t ctrl_x, int32_t ctrl_y);                     // Controlled Bezier curve

        // Mouse wheel
        bool mouseWheel(int32_t delta);

        // Mouse locking (masks physical input)
        bool lockMouseX(bool lock = true);
        bool lockMouseY(bool lock = true);
        bool lockMouseLeft(bool lock = true);
        bool lockMouseMiddle(bool lock = true);
        bool lockMouseRight(bool lock = true);
        bool lockMouseSide1(bool lock = true);
        bool lockMouseSide2(bool lock = true);

        // Get lock states
        bool isMouseXLocked();
        bool isMouseYLocked();
        bool isMouseLeftLocked();
        bool isMouseMiddleLocked();
        bool isMouseRightLocked();
        bool isMouseSide1Locked();
        bool isMouseSide2Locked();

        // Mouse input catching (for captured physical input)
        uint8_t catchMouseLeft();    // Returns captured left button presses
        uint8_t catchMouseMiddle();  // Returns captured middle button presses  
        uint8_t catchMouseRight();   // Returns captured right button presses
        uint8_t catchMouseSide1();   // Returns captured side1 button presses
        uint8_t catchMouseSide2();   // Returns captured side2 button presses

        // Button monitoring (v3.2 bitmask API)
        bool enableButtonMonitoring(bool enable = true);
        bool isButtonMonitoringEnabled();
        uint8_t getButtonMask(); // Returns bitmask of all button states

        // Mouse serial spoofing (v3.2 feature)
        std::string getMouseSerial();                    // Get current/original serial
        bool setMouseSerial(const std::string& serial); // Spoof mouse serial
        bool resetMouseSerial();                         // Reset to original

        // Device control
        bool setBaudRate(uint32_t baudRate);

        // Callbacks
        void setMouseButtonCallback(MouseButtonCallback callback);

        // Utility
        bool sendRawCommand(const std::string& command) const;
        std::string receiveRawResponse() const;

    private:
        // Implementation details
        class Impl;
        std::unique_ptr<Impl> m_impl;

        // Disable copy
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;
    };

    // Utility functions
    std::string mouseButtonToString(MouseButton button);
    MouseButton stringToMouseButton(const std::string& buttonName);

} // namespace makcu