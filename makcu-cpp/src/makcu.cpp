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
        Device::KeyboardCallback keyboardCallback;

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

    std::string Device::getSerialNumber() const {
        if (!m_impl->connected) {
            return "";
        }

        if (sendRawCommand("km.mac()\r")) {
            return receiveRawResponse();
        }

        return "";
    }

    // Keyboard control methods
    bool Device::keyDown(KeyCode key) {
        if (!m_impl->connected) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.down(" << static_cast<int>(key) << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::keyUp(KeyCode key) {
        if (!m_impl->connected) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.up(" << static_cast<int>(key) << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::keyPress(KeyCode key, uint32_t duration_ms) {
        if (!m_impl->connected) {
            return false;
        }

        std::ostringstream cmd;
        if (duration_ms > 0) {
            cmd << "km.press(" << static_cast<int>(key) << "," << duration_ms << ")\r";
        }
        else {
            cmd << "km.press(" << static_cast<int>(key) << ")\r";
        }
        return sendRawCommand(cmd.str());
    }

    bool Device::multiKeyDown(const std::vector<KeyCode>& keys) {
        if (!m_impl->connected || keys.empty()) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.multidown(";
        for (size_t i = 0; i < keys.size(); ++i) {
            cmd << static_cast<int>(keys[i]);
            if (i < keys.size() - 1) {
                cmd << ",";
            }
        }
        cmd << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::multiKeyUp(const std::vector<KeyCode>& keys) {
        if (!m_impl->connected || keys.empty()) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.multiup(";
        for (size_t i = 0; i < keys.size(); ++i) {
            cmd << static_cast<int>(keys[i]);
            if (i < keys.size() - 1) {
                cmd << ",";
            }
        }
        cmd << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::multiKeyPress(const std::vector<KeyCode>& keys, uint32_t duration_ms) {
        if (!m_impl->connected || keys.empty()) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.multipress(";
        for (size_t i = 0; i < keys.size(); ++i) {
            cmd << static_cast<int>(keys[i]);
            if (i < keys.size() - 1) {
                cmd << ",";
            }
        }
        if (duration_ms > 0) {
            cmd << "," << duration_ms;
        }
        cmd << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::typeString(const std::string& text) {
        if (!m_impl->connected || text.empty()) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.string(\"" << text << "\")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::isKeyDown(KeyCode key) {
        if (!m_impl->connected) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.isdown(" << static_cast<int>(key) << ")\r";
        if (sendRawCommand(cmd.str())) {
            auto response = receiveRawResponse();
            return response.find("1") != std::string::npos || response.find("3") != std::string::npos;
        }
        return false;
    }

    // Mouse control methods
    bool Device::mouseDown(MouseButton button) {
        if (!m_impl->connected) {
            return false;
        }

        std::string cmd;
        switch (button) {
        case MouseButton::LEFT:   cmd = "km.left(1)\r"; break;
        case MouseButton::RIGHT:  cmd = "km.right(1)\r"; break;
        case MouseButton::MIDDLE: cmd = "km.middle(1)\r"; break;
        case MouseButton::SIDE4:  cmd = "km.side1(1)\r"; break;
        case MouseButton::SIDE5:  cmd = "km.side2(1)\r"; break;
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
        case MouseButton::SIDE4:  cmd = "km.side1(0)\r"; break;
        case MouseButton::SIDE5:  cmd = "km.side2(0)\r"; break;
        }
        return sendRawCommand(cmd);
    }

    bool Device::mouseClick(MouseButton button, uint32_t count) {
        if (!m_impl->connected) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.click(" << static_cast<int>(button) << "," << count << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::mouseMove(int32_t x, int32_t y) {
        if (!m_impl->connected) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.move(" << x << "," << y << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::mouseMoveTo(int32_t x, int32_t y) {
        if (!m_impl->connected) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.moveto(" << x << "," << y << ")\r";
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

    MouseButtonStates Device::getMouseButtonStates() {
        MouseButtonStates states;
        return states;
    }

    bool Device::mouseSetPosition(int32_t x, int32_t y) {
        return mouseMoveTo(x, y);
    }

    std::pair<int32_t, int32_t> Device::mouseGetPosition() {
        return { 0, 0 };
    }

    bool Device::mouseCalibrate() {
        if (!m_impl->connected) {
            return false;
        }

        return sendRawCommand("km.zero()\r");
    }

    bool Device::mouseSetScreenBounds(int32_t width, int32_t height) {
        if (!m_impl->connected) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.screen(" << width << "," << height << ")\r";
        return sendRawCommand(cmd.str());
    }

    bool Device::reset() {
        if (!m_impl->connected) {
            return false;
        }

        return sendRawCommand("km.init()\r");
    }

    bool Device::setBaudRate(uint32_t baudRate) {
        if (!m_impl->connected) {
            return false;
        }

        return m_impl->serialPort->setBaudRate(baudRate);
    }

    bool Device::enableButtonMonitoring(bool enable) {
        if (!m_impl->connected) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.buttons(" << (enable ? 1 : 0) << ")\r";
        return sendRawCommand(cmd.str());
    }

    void Device::setMouseButtonCallback(MouseButtonCallback callback) {
        m_impl->mouseButtonCallback = callback;
    }

    void Device::setKeyboardCallback(KeyboardCallback callback) {
        m_impl->keyboardCallback = callback;
    }

    bool Device::delay(uint32_t milliseconds) {
        if (!m_impl->connected) {
            return false;
        }

        std::ostringstream cmd;
        cmd << "km.delay(" << milliseconds << ")\r";
        return sendRawCommand(cmd.str());
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
    std::string keyCodeToString(KeyCode key) {
        switch (key) {
        case KeyCode::KEY_A: return "A";
        case KeyCode::KEY_B: return "B";
        case KeyCode::KEY_C: return "C";
        case KeyCode::KEY_D: return "D";
        case KeyCode::KEY_E: return "E";
        case KeyCode::KEY_F: return "F";
        case KeyCode::KEY_G: return "G";
        case KeyCode::KEY_H: return "H";
        case KeyCode::KEY_I: return "I";
        case KeyCode::KEY_J: return "J";
        case KeyCode::KEY_K: return "K";
        case KeyCode::KEY_L: return "L";
        case KeyCode::KEY_M: return "M";
        case KeyCode::KEY_N: return "N";
        case KeyCode::KEY_O: return "O";
        case KeyCode::KEY_P: return "P";
        case KeyCode::KEY_Q: return "Q";
        case KeyCode::KEY_R: return "R";
        case KeyCode::KEY_S: return "S";
        case KeyCode::KEY_T: return "T";
        case KeyCode::KEY_U: return "U";
        case KeyCode::KEY_V: return "V";
        case KeyCode::KEY_W: return "W";
        case KeyCode::KEY_X: return "X";
        case KeyCode::KEY_Y: return "Y";
        case KeyCode::KEY_Z: return "Z";
        case KeyCode::KEY_SPACEBAR: return "SPACE";
        case KeyCode::KEY_ENTER: return "ENTER";
        case KeyCode::KEY_ESCAPE: return "ESCAPE";
        default: return "UNKNOWN";
        }
    }

    KeyCode stringToKeyCode(const std::string& keyName) {
        std::string upper = keyName;
        std::transform(upper.begin(), upper.end(), upper.begin(),
            [](unsigned char c) { return std::toupper(c); });

        if (upper == "A") return KeyCode::KEY_A;
        if (upper == "B") return KeyCode::KEY_B;
        if (upper == "C") return KeyCode::KEY_C;
        if (upper == "D") return KeyCode::KEY_D;
        if (upper == "E") return KeyCode::KEY_E;
        if (upper == "F") return KeyCode::KEY_F;
        if (upper == "G") return KeyCode::KEY_G;
        if (upper == "H") return KeyCode::KEY_H;
        if (upper == "I") return KeyCode::KEY_I;
        if (upper == "J") return KeyCode::KEY_J;
        if (upper == "K") return KeyCode::KEY_K;
        if (upper == "L") return KeyCode::KEY_L;
        if (upper == "M") return KeyCode::KEY_M;
        if (upper == "N") return KeyCode::KEY_N;
        if (upper == "O") return KeyCode::KEY_O;
        if (upper == "P") return KeyCode::KEY_P;
        if (upper == "Q") return KeyCode::KEY_Q;
        if (upper == "R") return KeyCode::KEY_R;
        if (upper == "S") return KeyCode::KEY_S;
        if (upper == "T") return KeyCode::KEY_T;
        if (upper == "U") return KeyCode::KEY_U;
        if (upper == "V") return KeyCode::KEY_V;
        if (upper == "W") return KeyCode::KEY_W;
        if (upper == "X") return KeyCode::KEY_X;
        if (upper == "Y") return KeyCode::KEY_Y;
        if (upper == "Z") return KeyCode::KEY_Z;
        if (upper == "SPACE") return KeyCode::KEY_SPACEBAR;
        if (upper == "ENTER") return KeyCode::KEY_ENTER;
        if (upper == "ESCAPE") return KeyCode::KEY_ESCAPE;

        return KeyCode::KEY_A; // Default fallback
    }

    std::string mouseButtonToString(MouseButton button) {
        switch (button) {
        case MouseButton::LEFT: return "LEFT";
        case MouseButton::RIGHT: return "RIGHT";
        case MouseButton::MIDDLE: return "MIDDLE";
        case MouseButton::SIDE4: return "SIDE4";
        case MouseButton::SIDE5: return "SIDE5";
        }
        return "UNKNOWN";
    }

} // namespace makcus