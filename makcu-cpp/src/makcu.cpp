#include "../include/makcu.h"
#include "../include/serialport.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <atomic>
#include <mutex>

namespace makcu {

    // Constants
    constexpr uint16_t MAKCU_VID = 0x1A86;
    constexpr uint16_t MAKCU_PID = 0x55D3;
    constexpr const char* TARGET_DESC = "USB-Enhanced-SERIAL CH343";
    constexpr const char* DEFAULT_NAME = "USB-SERIAL CH340";
    constexpr uint32_t INITIAL_BAUD_RATE = 115200;
    constexpr uint32_t HIGH_SPEED_BAUD_RATE = 4000000;

    // Baud rate change command
    const std::vector<uint8_t> BAUD_CHANGE_COMMAND = { 0xDE, 0xAD, 0x05, 0x00, 0xA5, 0x00, 0x09, 0x3D, 0x00 };

    // PIMPL implementation
    class Device::Impl {
    public:
        std::unique_ptr<SerialPort> serialPort;
        DeviceInfo deviceInfo;
        ConnectionStatus status;
        std::atomic<bool> connected;
        std::atomic<bool> monitoring;
        mutable std::mutex mutex;
        std::thread monitorThread;
        Device::MouseButtonCallback mouseButtonCallback;

        Impl()
            : serialPort(std::make_unique<SerialPort>())
            , status(ConnectionStatus::DISCONNECTED)
            , connected(false)
            , monitoring(false)
        {
            deviceInfo.isConnected = false;
        }

        ~Impl() {
            stopMonitoring();
        }

        bool switchToHighSpeedMode() {
            if (!serialPort->isOpen()) {
                std::cout << "Error: Serial port not open for baud rate switch\n";
                return false;
            }

            std::cout << "Sending baud rate change command...\n";

            // Send baud rate change command
            if (!serialPort->write(BAUD_CHANGE_COMMAND)) {
                std::cout << "Error: Failed to send baud rate change command\n";
                return false;
            }

            if (!serialPort->flush()) {
                std::cout << "Error: Failed to flush after baud rate command\n";
                return false;
            }

            std::cout << "Baud rate command sent successfully\n";

            // Close and reopen at high speed
            std::string portName = serialPort->getPortName();
            serialPort->close();

            std::cout << "Port closed, waiting before reopening...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            std::cout << "Reopening port at " << HIGH_SPEED_BAUD_RATE << " baud...\n";
            if (!serialPort->open(portName, HIGH_SPEED_BAUD_RATE)) {
                std::cout << "Error: Failed to reopen port at high speed\n";
                return false;
            }

            std::cout << "Successfully switched to high-speed mode\n";
            return true;
        }

        bool initializeDevice() {
            if (!serialPort->isOpen()) {
                std::cout << "Error: Serial port not open for initialization\n";
                return false;
            }

            std::cout << "Initializing device...\n";

            // Wait a bit for device to be ready (like Python script does)
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));

            // Enable button monitoring
            std::string cmd = "km.buttons(1)\r";
            std::cout << "Sending initialization command: " << cmd;

            if (!serialPort->write(cmd)) {
                std::cout << "Error: Failed to send initialization command\n";
                return false;
            }

            if (!serialPort->flush()) {
                std::cout << "Error: Failed to flush after initialization\n";
                return false;
            }

            // Wait a bit like Python script
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            std::cout << "Device initialized successfully\n";
            return true;
        }

        void startMonitoring() {
            if (monitoring) {
                return;
            }

            std::cout << "Starting monitoring thread...\n";
            monitoring = true;
            monitorThread = std::thread(&Impl::monitoringLoop, this);
        }

        void stopMonitoring() {
            if (!monitoring) {
                return;
            }

            std::cout << "Stopping monitoring thread...\n";
            monitoring = false;
            if (monitorThread.joinable()) {
                monitorThread.join();
            }
        }

        void monitoringLoop() {
            uint8_t lastValue = 0;
            std::cout << "Monitoring loop started\n";

            while (monitoring && connected) {
                try {
                    if (serialPort->available() > 0) {
                        uint8_t byte;
                        if (serialPort->readByte(byte)) {
                            if (byte != lastValue) {
                                processButtonData(byte);
                                lastValue = byte;
                            }
                        }
                    }
                }
                catch (const std::exception& e) {
                    std::cout << "Error in monitoring loop: " << e.what() << "\n";
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }

            std::cout << "Monitoring loop ended\n";
        }

        bool processButtonData(uint8_t data) {
            if (!mouseButtonCallback) {
                return true;
            }

            // Check each mouse button bit
            for (int bit = 0; bit < 5; ++bit) {
                bool isPressed = (data & (1 << bit)) != 0;
                MouseButton button = static_cast<MouseButton>(bit);
                mouseButtonCallback(button, isPressed);
            }

            return true;
        }
    };

    // Device implementation
    Device::Device() : m_impl(std::make_unique<Impl>()) {
    }

    Device::~Device() {
        disconnect();
    }

    std::vector<DeviceInfo> Device::findDevices() {
        std::vector<DeviceInfo> devices;
        auto ports = SerialPort::findMakcuPorts();

        std::cout << "SerialPort::findMakcuPorts() returned " << ports.size() << " ports\n";

        for (const auto& port : ports) {
            std::cout << "Found MAKCU port: " << port << "\n";
            DeviceInfo info;
            info.port = port;
            info.description = TARGET_DESC;
            info.vid = MAKCU_VID;
            info.pid = MAKCU_PID;
            info.isConnected = false;
            devices.push_back(info);
        }

        return devices;
    }

    std::string Device::findFirstDevice() {
        auto devices = findDevices();
        if (!devices.empty()) {
            return devices[0].port;
        }
        return "";
    }

    bool Device::connect(const std::string& port) {
        std::lock_guard<std::mutex> lock(m_impl->mutex);

        if (m_impl->connected) {
            std::cout << "Already connected\n";
            return true;
        }

        std::string targetPort = port;
        if (targetPort.empty()) {
            std::cout << "No port specified, searching for device...\n";
            targetPort = findFirstDevice();
            if (targetPort.empty()) {
                std::cout << "No MAKCU device found\n";
                m_impl->status = ConnectionStatus::CONNECTION_ERROR;
                return false;
            }
        }

        std::cout << "Attempting to connect to " << targetPort << "\n";
        m_impl->status = ConnectionStatus::CONNECTING;

        // Try to open at initial baud rate
        std::cout << "Opening port at " << INITIAL_BAUD_RATE << " baud...\n";
        if (!m_impl->serialPort->open(targetPort, INITIAL_BAUD_RATE)) {
            std::cout << "Failed to open port " << targetPort << " at " << INITIAL_BAUD_RATE << " baud\n";
            m_impl->status = ConnectionStatus::CONNECTION_ERROR;
            return false;
        }

        std::cout << "Port opened successfully\n";

        // Switch to high-speed mode
        std::cout << "Switching to high-speed mode...\n";
        if (!m_impl->switchToHighSpeedMode()) {
            std::cout << "Failed to switch to high-speed mode\n";
            m_impl->serialPort->close();
            m_impl->status = ConnectionStatus::CONNECTION_ERROR;
            return false;
        }

        // Initialize the device
        std::cout << "Initializing device...\n";
        if (!m_impl->initializeDevice()) {
            std::cout << "Failed to initialize device\n";
            m_impl->serialPort->close();
            m_impl->status = ConnectionStatus::CONNECTION_ERROR;
            return false;
        }

        // Update device info
        m_impl->deviceInfo.port = targetPort;
        m_impl->deviceInfo.description = TARGET_DESC;
        m_impl->deviceInfo.vid = MAKCU_VID;
        m_impl->deviceInfo.pid = MAKCU_PID;
        m_impl->deviceInfo.isConnected = true;

        m_impl->connected = true;
        m_impl->status = ConnectionStatus::CONNECTED;

        // Start monitoring thread
        m_impl->startMonitoring();

        std::cout << "Device connected successfully!\n";
        return true;
    }

    void Device::disconnect() {
        std::lock_guard<std::mutex> lock(m_impl->mutex);

        if (!m_impl->connected) {
            return;
        }

        std::cout << "Disconnecting device...\n";
        m_impl->stopMonitoring();
        m_impl->serialPort->close();

        m_impl->connected = false;
        m_impl->status = ConnectionStatus::DISCONNECTED;
        m_impl->deviceInfo.isConnected = false;
        std::cout << "Device disconnected\n";
    }

    bool Device::isConnected() const {
        return m_impl->connected;
    }

    ConnectionStatus Device::getStatus() const {
        return m_impl->status;
    }

    DeviceInfo Device::getDeviceInfo() const {
        return m_impl->deviceInfo;
    }

    std::string Device::getVersion() const {
        if (!m_impl->connected) {
            return "";
        }

        if (sendRawCommand("km.version()\r")) {
            return receiveRawResponse();
        }

        return "";
    }

    // Mouse button control methods
    bool Device::mouseDown(MouseButton button) {
        if (!m_impl->connected) {
            return false;
        }

        std::string cmd;
        switch (button) {
        case MouseButton::LEFT:   cmd = "km.left(1)\r"; break;
        case MouseButton::RIGHT:  cmd = "km.right(1)\r"; break;
        case MouseButton::MIDDLE: cmd = "km.middle(1)\r"; break;
        default: return false; // Side buttons not supported based on test results
        }
        return sendRawCommand(cmd);
    }

    bool Device::mouseUp(MouseButton button) {
        if (!m_impl->connected) {
            return false;
        }

        std::string cmd;
        switch (button) {
        case MouseButton::LEFT:   cmd = "km.left(0)\r"; break;
        case MouseButton::RIGHT:  cmd = "km.right(0)\r"; break;
        case MouseButton::MIDDLE: cmd = "km.middle(0)\r"; break;
        default: return false; // Side buttons not supported based on test results
        }
        return sendRawCommand(cmd);
    }

    bool Device::mouseButtonState(MouseButton button) {
        if (!m_impl->connected) {
            return false;
        }

        std::string cmd;
        switch (button) {
        case MouseButton::LEFT:   cmd = "km.left()\r"; break;
        case MouseButton::RIGHT:  cmd = "km.right()\r"; break;
        case MouseButton::MIDDLE: cmd = "km.middle()\r"; break;
        default: return false; // Side buttons not supported based on test results
        }

        if (sendRawCommand(cmd)) {
            auto response = receiveRawResponse();
            return response.find("1") != std::string::npos;
        }
        return false;
    }

    // Mouse movement methods (v3.2 enhanced support)
    bool Device::mouseMove(int32_t x, int32_t y) {
        if (!m_impl->connected) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.move(" << x << "," << y << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::mouseMoveSmooth(int32_t x, int32_t y, uint32_t segments) {
        if (!m_impl->connected) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.move(" << x << "," << y << "," << segments << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::mouseMoveBezier(int32_t x, int32_t y, uint32_t segments, int32_t ctrl_x, int32_t ctrl_y) {
        if (!m_impl->connected) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.move(" << x << "," << y << "," << segments << "," << ctrl_x << "," << ctrl_y << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::mouseWheel(int32_t delta) {
        if (!m_impl->connected) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.wheel(" << delta << ")\r";
        return sendRawCommand(cmd.str());
    }

    // Mouse locking methods
    bool Device::lockMouseX(bool lock) {
        if (!m_impl->connected) return false;
        std::ostringstream cmd;
        cmd << "km.lock_mx(" << (lock ? 1 : 0) << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::lockMouseY(bool lock) {
        if (!m_impl->connected) return false;
        std::ostringstream cmd;
        cmd << "km.lock_my(" << (lock ? 1 : 0) << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::lockMouseLeft(bool lock) {
        if (!m_impl->connected) return false;
        std::ostringstream cmd;
        cmd << "km.lock_ml(" << (lock ? 1 : 0) << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::lockMouseMiddle(bool lock) {
        if (!m_impl->connected) return false;
        std::ostringstream cmd;
        cmd << "km.lock_mm(" << (lock ? 1 : 0) << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::lockMouseRight(bool lock) {
        if (!m_impl->connected) return false;
        std::ostringstream cmd;
        cmd << "km.lock_mr(" << (lock ? 1 : 0) << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::lockMouseSide1(bool lock) {
        if (!m_impl->connected) return false;
        std::ostringstream cmd;
        cmd << "km.lock_ms1(" << (lock ? 1 : 0) << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::lockMouseSide2(bool lock) {
        if (!m_impl->connected) return false;
        std::ostringstream cmd;
        cmd << "km.lock_ms2(" << (lock ? 1 : 0) << ")\r";
        return sendRawCommand(cmd.str());
    }

    // Get lock states
    bool Device::isMouseXLocked() {
        if (!m_impl->connected) return false;
        if (sendRawCommand("km.lock_mx()\r")) {
            auto response = receiveRawResponse();
            return response.find("1") != std::string::npos;
        }
        return false;
    }

    bool Device::isMouseYLocked() {
        if (!m_impl->connected) return false;
        if (sendRawCommand("km.lock_my()\r")) {
            auto response = receiveRawResponse();
            return response.find("1") != std::string::npos;
        }
        return false;
    }

    bool Device::isMouseLeftLocked() {
        if (!m_impl->connected) return false;
        if (sendRawCommand("km.lock_ml()\r")) {
            auto response = receiveRawResponse();
            return response.find("1") != std::string::npos;
        }
        return false;
    }

    bool Device::isMouseMiddleLocked() {
        if (!m_impl->connected) return false;
        if (sendRawCommand("km.lock_mm()\r")) {
            auto response = receiveRawResponse();
            return response.find("1") != std::string::npos;
        }
        return false;
    }

    bool Device::isMouseRightLocked() {
        if (!m_impl->connected) return false;
        if (sendRawCommand("km.lock_mr()\r")) {
            auto response = receiveRawResponse();
            return response.find("1") != std::string::npos;
        }
        return false;
    }

    bool Device::isMouseSide1Locked() {
        if (!m_impl->connected) return false;
        if (sendRawCommand("km.lock_ms1()\r")) {
            auto response = receiveRawResponse();
            return response.find("1") != std::string::npos;
        }
        return false;
    }

    bool Device::isMouseSide2Locked() {
        if (!m_impl->connected) return false;
        if (sendRawCommand("km.lock_ms2()\r")) {
            auto response = receiveRawResponse();
            return response.find("1") != std::string::npos;
        }
        return false;
    }

    // Mouse input catching methods
    uint8_t Device::catchMouseLeft() {
        if (!m_impl->connected) return 0;
        if (sendRawCommand("km.catch_ml()\r")) {
            auto response = receiveRawResponse();
            try {
                return static_cast<uint8_t>(std::stoi(response));
            }
            catch (...) {
                return 0;
            }
        }
        return 0;
    }

    uint8_t Device::catchMouseMiddle() {
        if (!m_impl->connected) return 0;
        if (sendRawCommand("km.catch_mm()\r")) {
            auto response = receiveRawResponse();
            try {
                return static_cast<uint8_t>(std::stoi(response));
            }
            catch (...) {
                return 0;
            }
        }
        return 0;
    }

    uint8_t Device::catchMouseRight() {
        if (!m_impl->connected) return 0;
        if (sendRawCommand("km.catch_mr()\r")) {
            auto response = receiveRawResponse();
            try {
                return static_cast<uint8_t>(std::stoi(response));
            }
            catch (...) {
                return 0;
            }
        }
        return 0;
    }

    uint8_t Device::catchMouseSide1() {
        if (!m_impl->connected) return 0;
        if (sendRawCommand("km.catch_ms1()\r")) {
            auto response = receiveRawResponse();
            try {
                return static_cast<uint8_t>(std::stoi(response));
            }
            catch (...) {
                return 0;
            }
        }
        return 0;
    }

    uint8_t Device::catchMouseSide2() {
        if (!m_impl->connected) return 0;
        if (sendRawCommand("km.catch_ms2()\r")) {
            auto response = receiveRawResponse();
            try {
                return static_cast<uint8_t>(std::stoi(response));
            }
            catch (...) {
                return 0;
            }
        }
        return 0;
    }

    // Button monitoring methods (v3.2 bitmask API)
    bool Device::enableButtonMonitoring(bool enable) {
        if (!m_impl->connected) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.buttons(" << (enable ? 1 : 0) << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::isButtonMonitoringEnabled() {
        if (!m_impl->connected) return false;
        if (sendRawCommand("km.buttons()\r")) {
            auto response = receiveRawResponse();
            return response.find("1") != std::string::npos;
        }
        return false;
    }

    uint8_t Device::getButtonMask() {
        // This would need to be implemented by reading the real-time button data
        // from the monitoring loop or by sending a specific command
        // For now, returning 0 as placeholder
        return 0;
    }

    // Mouse serial spoofing methods (v3.2 feature)
    std::string Device::getMouseSerial() {
        if (!m_impl->connected) return "";
        if (sendRawCommand("km.serial()\r")) {
            return receiveRawResponse();
        }
        return "";
    }

    bool Device::setMouseSerial(const std::string& serial) {
        if (!m_impl->connected) return false;
        std::ostringstream cmd;
        cmd << "km.serial('" << serial << "')\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::resetMouseSerial() {
        if (!m_impl->connected) return false;
        return sendRawCommand("km.serial(0)\r");
    }

    bool Device::setBaudRate(uint32_t baudRate) {
        if (!m_impl->connected) {
            return false;
        }

        return m_impl->serialPort->setBaudRate(baudRate);
    }

    void Device::setMouseButtonCallback(MouseButtonCallback callback) {
        m_impl->mouseButtonCallback = callback;
    }

    bool Device::sendRawCommand(const std::string& command) const {
        if (!m_impl->connected) {
            return false;
        }

        return m_impl->serialPort->write(command);
    }

    std::string Device::receiveRawResponse() const {
        if (!m_impl->connected) {
            return "";
        }

        // Wait a bit for response
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return m_impl->serialPort->readString(1024);
    }

    // Utility functions
    std::string mouseButtonToString(MouseButton button) {
        switch (button) {
        case MouseButton::LEFT: return "LEFT";
        case MouseButton::RIGHT: return "RIGHT";
        case MouseButton::MIDDLE: return "MIDDLE";
        case MouseButton::SIDE1: return "SIDE1";
        case MouseButton::SIDE2: return "SIDE2";
        }
        return "UNKNOWN";
    }

    MouseButton stringToMouseButton(const std::string& buttonName) {
        std::string upper = buttonName;
        std::transform(upper.begin(), upper.end(), upper.begin(),
            [](unsigned char c) { return std::toupper(c); });

        if (upper == "LEFT") return MouseButton::LEFT;
        if (upper == "RIGHT") return MouseButton::RIGHT;
        if (upper == "MIDDLE") return MouseButton::MIDDLE;
        if (upper == "SIDE1") return MouseButton::SIDE1;
        if (upper == "SIDE2") return MouseButton::SIDE2;

        return MouseButton::LEFT; // Default fallback
    }

} // namespace makcu