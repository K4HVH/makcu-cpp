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

    enum class KeyCode : uint8_t {
        KEY_A = 0x04, KEY_B = 0x05, KEY_C = 0x06, KEY_D = 0x07,
        KEY_E = 0x08, KEY_F = 0x09, KEY_G = 0x0A, KEY_H = 0x0B,
        KEY_I = 0x0C, KEY_J = 0x0D, KEY_K = 0x0E, KEY_L = 0x0F,
        KEY_M = 0x10, KEY_N = 0x11, KEY_O = 0x12, KEY_P = 0x13,
        KEY_Q = 0x14, KEY_R = 0x15, KEY_S = 0x16, KEY_T = 0x17,
        KEY_U = 0x18, KEY_V = 0x19, KEY_W = 0x1A, KEY_X = 0x1B,
        KEY_Y = 0x1C, KEY_Z = 0x1D,
        
        KEY_1 = 0x1E, KEY_2 = 0x1F, KEY_3 = 0x20, KEY_4 = 0x21,
        KEY_5 = 0x22, KEY_6 = 0x23, KEY_7 = 0x24, KEY_8 = 0x25,
        KEY_9 = 0x26, KEY_0 = 0x27,
        
        KEY_ENTER = 0x28, KEY_ESCAPE = 0x29, KEY_BACKSPACE = 0x2A,
        KEY_TAB = 0x2B, KEY_SPACEBAR = 0x2C,
        
        KEY_LEFT_CTRL = 0xE0, KEY_LEFT_SHIFT = 0xE1, KEY_LEFT_ALT = 0xE2,
        KEY_LEFT_GUI = 0xE3, KEY_RIGHT_CTRL = 0xE4, KEY_RIGHT_SHIFT = 0xE5,
        KEY_RIGHT_ALT = 0xE6, KEY_RIGHT_GUI = 0xE7
    };

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
            : MakcuException("Connection error: " + message) {}
    };

    class CommandException : public MakcuException {
    public:
        explicit CommandException(const std::string& message) 
            : MakcuException("Command error: " + message) {}
    };

    // Main Device class
    class Device {
    public:
        // Callback types
        typedef std::function<void(MouseButton, bool)> MouseButtonCallback;
        typedef std::function<void(KeyCode, bool)> KeyboardCallback;

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
        std::string getSerialNumber() const;

        // Keyboard control
        bool keyDown(KeyCode key);
        bool keyUp(KeyCode key);
        bool keyPress(KeyCode key, uint32_t duration_ms = 0);
        bool multiKeyDown(const std::vector<KeyCode>& keys);
        bool multiKeyUp(const std::vector<KeyCode>& keys);
        bool multiKeyPress(const std::vector<KeyCode>& keys, uint32_t duration_ms = 0);
        bool typeString(const std::string& text);
        bool isKeyDown(KeyCode key);

        // Mouse control
        bool mouseDown(MouseButton button);
        bool mouseUp(MouseButton button);
        bool mouseClick(MouseButton button, uint32_t count = 1);
        bool mouseMove(int32_t x, int32_t y);
        bool mouseMoveTo(int32_t x, int32_t y);
        bool mouseWheel(int32_t delta);
        MouseButtonStates getMouseButtonStates();

        // Advanced mouse
        bool mouseSetPosition(int32_t x, int32_t y);
        std::pair<int32_t, int32_t> mouseGetPosition();
        bool mouseCalibrate();
        bool mouseSetScreenBounds(int32_t width, int32_t height);

        // Device control
        bool reset();
        bool setBaudRate(uint32_t baudRate);
        bool enableButtonMonitoring(bool enable = true);

        // Callbacks
        void setMouseButtonCallback(MouseButtonCallback callback);
        void setKeyboardCallback(KeyboardCallback callback);

        // Utility
        bool delay(uint32_t milliseconds);
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
    std::string keyCodeToString(KeyCode key);
    KeyCode stringToKeyCode(const std::string& keyName);
    std::string mouseButtonToString(MouseButton button);

} // namespace makcu
