#include "../include/serialport.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>
#include <chrono>

#ifdef _WIN32
#include <setupapi.h>
#include <devguid.h>
#include <cfgmgr32.h>
#pragma comment(lib, "setupapi.lib")
#endif

namespace makcu {

    SerialPort::SerialPort()
        : m_baudRate(115200)
        , m_timeout(100)  // Reduced from 1000ms
        , m_isOpen(false)
#ifdef _WIN32
        , m_handle(INVALID_HANDLE_VALUE)
#else
        , m_fd(-1)
#endif
    {
#ifdef _WIN32
        memset(&m_dcb, 0, sizeof(m_dcb));
        memset(&m_timeouts, 0, sizeof(m_timeouts));
#endif
    }

    SerialPort::~SerialPort() {
        close();
    }

    bool SerialPort::open(const std::string& port, uint32_t baudRate) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_isOpen) {
            close();
        }

        m_portName = port;
        m_baudRate = baudRate;

#ifdef _WIN32
        std::string fullPortName = "\\\\.\\" + port;

        m_handle = CreateFileA(
            fullPortName.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (m_handle == INVALID_HANDLE_VALUE) {
            return false;
        }

        if (!configurePort()) {
            CloseHandle(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
            return false;
        }

        m_isOpen = true;

        // Start high-performance listener thread
        m_stopListener = false;
        m_listenerThread = std::thread(&SerialPort::listenerLoop, this);

        return true;
#else
        return false; // Linux implementation would go here
#endif
    }

    void SerialPort::close() {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (!m_isOpen) {
            return;
        }

        // Stop listener thread
        m_stopListener = true;
        if (m_listenerThread.joinable()) {
            m_listenerThread.join();
        }

        // Cancel all pending commands
        {
            std::lock_guard<std::mutex> cmdLock(m_commandMutex);
            for (auto& [id, cmd] : m_pendingCommands) {
                try {
                    cmd->promise.set_exception(std::make_exception_ptr(
                        std::runtime_error("Connection closed")));
                }
                catch (...) {
                    // Promise already set
                }
            }
            m_pendingCommands.clear();
        }

#ifdef _WIN32
        if (m_handle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
        }
#else
        if (m_fd >= 0) {
            ::close(m_fd);
            m_fd = -1;
        }
#endif

        m_isOpen = false;
    }

    bool SerialPort::isOpen() const {
        return m_isOpen;
    }

    std::future<std::string> SerialPort::sendTrackedCommand(const std::string& command,
        bool expectResponse,
        std::chrono::milliseconds timeout) {
        if (!m_isOpen) {
            std::promise<std::string> promise;
            promise.set_exception(std::make_exception_ptr(
                std::runtime_error("Port not open")));
            return promise.get_future();
        }

        int cmdId = generateCommandId();
        auto pendingCmd = std::make_unique<PendingCommand>(cmdId, command, expectResponse, timeout);
        auto future = pendingCmd->promise.get_future();

        // Store pending command
        {
            std::lock_guard<std::mutex> lock(m_commandMutex);
            m_pendingCommands[cmdId] = std::move(pendingCmd);
        }

        // Send command with ID tracking
        std::string trackedCommand = expectResponse ?
            command + "#" + std::to_string(cmdId) + "\r\n" :
            command + "\r\n";

#ifdef _WIN32
        DWORD bytesWritten = 0;
        bool success = WriteFile(m_handle, trackedCommand.c_str(),
            static_cast<DWORD>(trackedCommand.length()),
            &bytesWritten, nullptr);

        if (!success || bytesWritten != trackedCommand.length()) {
            std::lock_guard<std::mutex> lock(m_commandMutex);
            auto it = m_pendingCommands.find(cmdId);
            if (it != m_pendingCommands.end()) {
                try {
                    it->second->promise.set_exception(std::make_exception_ptr(
                        std::runtime_error("Write failed")));
                }
                catch (...) {
                    // Promise already set
                }
                m_pendingCommands.erase(it);
            }
        }

        FlushFileBuffers(m_handle);
#endif

        return future;
    }

    bool SerialPort::sendCommand(const std::string& command) {
        if (!m_isOpen) {
            return false;
        }

        std::string fullCommand = command + "\r\n";

#ifdef _WIN32
        DWORD bytesWritten = 0;
        bool success = WriteFile(m_handle, fullCommand.c_str(),
            static_cast<DWORD>(fullCommand.length()),
            &bytesWritten, nullptr);

        if (success && bytesWritten == fullCommand.length()) {
            FlushFileBuffers(m_handle);
            return true;
        }
#endif

        return false;
    }

    void SerialPort::listenerLoop() {
        // Optimized read buffers
        std::vector<uint8_t> readBuffer(BUFFER_SIZE);
        std::vector<uint8_t> lineBuffer(LINE_BUFFER_SIZE);
        size_t linePos = 0;

        auto lastCleanup = std::chrono::steady_clock::now();
        constexpr auto cleanupInterval = std::chrono::milliseconds(50);

        while (!m_stopListener && m_isOpen.load()) {
            try {
#ifdef _WIN32
                DWORD bytesAvailable = 0;
                COMSTAT comStat;
                DWORD errors;

                if (!ClearCommError(m_handle, &errors, &comStat)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }

                bytesAvailable = comStat.cbInQue;
                if (bytesAvailable == 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds(500));
                    continue;
                }

                DWORD bytesToRead = std::min<DWORD>(bytesAvailable, static_cast<DWORD>(BUFFER_SIZE));
                DWORD bytesRead = 0;

                if (!ReadFile(m_handle, readBuffer.data(), bytesToRead, &bytesRead, nullptr)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }

                // Process each byte efficiently
                for (DWORD i = 0; i < bytesRead; ++i) {
                    uint8_t byte = readBuffer[i];

                    // Handle button data (non-printable characters < 32, except CR/LF)
                    if (byte < 32 && byte != 0x0D && byte != 0x0A) {
                        handleButtonData(byte);
                    }
                    else {
                        // Handle text response data
                        if (byte == 0x0A) { // Line feed
                            if (linePos > 0) {
                                std::string line(lineBuffer.begin(), lineBuffer.begin() + linePos);
                                linePos = 0;
                                if (!line.empty()) {
                                    processResponse(line);
                                }
                            }
                        }
                        else if (byte != 0x0D) { // Ignore carriage return
                            if (linePos < LINE_BUFFER_SIZE - 1) {
                                lineBuffer[linePos++] = byte;
                            }
                        }
                    }
                }
#endif

                // Periodic cleanup of timed-out commands
                auto now = std::chrono::steady_clock::now();
                if (now - lastCleanup > cleanupInterval) {
                    cleanupTimedOutCommands();
                    lastCleanup = now;
                }

            }
            catch (const std::exception&) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    void SerialPort::handleButtonData(uint8_t data) {
        uint8_t lastMask = m_lastButtonMask.load();
        if (data == lastMask) {
            return; // No change
        }

        m_lastButtonMask.store(data);

        if (m_buttonCallback) {
            // Only process changed bits
            uint8_t changedBits = data ^ lastMask;
            for (int bit = 0; bit < 5; ++bit) {
                if (changedBits & (1 << bit)) {
                    bool isPressed = data & (1 << bit);
                    try {
                        m_buttonCallback(bit, isPressed);
                    }
                    catch (...) {
                        // Ignore callback exceptions
                    }
                }
            }
        }
    }

    void SerialPort::processResponse(const std::string& response) {
        // Remove ">>> " prefix if present
        std::string content = response;
        if (content.substr(0, 4) == ">>> ") {
            content = content.substr(4);
        }

        // Check for command ID correlation
        size_t hashPos = content.find('#');
        if (hashPos != std::string::npos) {
            // Extract command ID
            std::string idStr = content.substr(hashPos + 1);
            size_t colonPos = idStr.find(':');
            if (colonPos != std::string::npos) {
                try {
                    int cmdId = std::stoi(idStr.substr(0, colonPos));
                    std::string result = idStr.substr(colonPos + 1);

                    std::lock_guard<std::mutex> lock(m_commandMutex);
                    auto it = m_pendingCommands.find(cmdId);
                    if (it != m_pendingCommands.end()) {
                        try {
                            it->second->promise.set_value(result);
                        }
                        catch (...) {
                            // Promise already set
                        }
                        m_pendingCommands.erase(it);
                    }
                    return;
                }
                catch (...) {
                    // Failed to parse ID, treat as normal response
                }
            }
        }

        // Handle untracked response (oldest pending command)
        std::lock_guard<std::mutex> lock(m_commandMutex);
        if (!m_pendingCommands.empty()) {
            auto it = m_pendingCommands.begin();
            try {
                it->second->promise.set_value(content);
            }
            catch (...) {
                // Promise already set
            }
            m_pendingCommands.erase(it);
        }
    }

    void SerialPort::cleanupTimedOutCommands() {
        auto now = std::chrono::steady_clock::now();

        std::lock_guard<std::mutex> lock(m_commandMutex);
        auto it = m_pendingCommands.begin();
        while (it != m_pendingCommands.end()) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - it->second->timestamp);

            if (elapsed > it->second->timeout) {
                try {
                    it->second->promise.set_exception(std::make_exception_ptr(
                        std::runtime_error("Command timeout")));
                }
                catch (...) {
                    // Promise already set
                }
                it = m_pendingCommands.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    int SerialPort::generateCommandId() {
        return (m_commandCounter.fetch_add(1) % 10000) + 1;
    }

    bool SerialPort::configurePort() {
#ifdef _WIN32
        m_dcb.DCBlength = sizeof(DCB);

        if (!GetCommState(m_handle, &m_dcb)) {
            return false;
        }

        m_dcb.BaudRate = m_baudRate;
        m_dcb.ByteSize = 8;
        m_dcb.Parity = NOPARITY;
        m_dcb.StopBits = ONESTOPBIT;
        m_dcb.fBinary = TRUE;
        m_dcb.fParity = FALSE;
        m_dcb.fOutxCtsFlow = FALSE;
        m_dcb.fOutxDsrFlow = FALSE;
        m_dcb.fDtrControl = DTR_CONTROL_DISABLE;
        m_dcb.fDsrSensitivity = FALSE;
        m_dcb.fTXContinueOnXoff = FALSE;
        m_dcb.fOutX = FALSE;
        m_dcb.fInX = FALSE;
        m_dcb.fErrorChar = FALSE;
        m_dcb.fNull = FALSE;
        m_dcb.fRtsControl = RTS_CONTROL_DISABLE;
        m_dcb.fAbortOnError = FALSE;

        if (!SetCommState(m_handle, &m_dcb)) {
            return false;
        }

        updateTimeouts();
        return true;
#else
        return false;
#endif
    }

    void SerialPort::updateTimeouts() {
#ifdef _WIN32
        // Gaming-optimized timeouts - much faster than original
        m_timeouts.ReadIntervalTimeout = 1;          // 1ms between bytes
        m_timeouts.ReadTotalTimeoutConstant = 10;    // 10ms total read timeout
        m_timeouts.ReadTotalTimeoutMultiplier = 1;   // 1ms per byte
        m_timeouts.WriteTotalTimeoutConstant = 10;   // 10ms write timeout
        m_timeouts.WriteTotalTimeoutMultiplier = 1;  // 1ms per byte

        SetCommTimeouts(m_handle, &m_timeouts);
#endif
    }

    void SerialPort::setButtonCallback(ButtonCallback callback) {
        m_buttonCallback = callback;
    }

    // Legacy compatibility methods
    bool SerialPort::setBaudRate(uint32_t baudRate) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_isOpen) {
            m_baudRate = baudRate;
            return true;
        }
        m_baudRate = baudRate;
#ifdef _WIN32
        m_dcb.BaudRate = baudRate;
        return SetCommState(m_handle, &m_dcb) != 0;
#else
        return false;
#endif
    }

    uint32_t SerialPort::getBaudRate() const {
        return m_baudRate;
    }

    std::string SerialPort::getPortName() const {
        return m_portName;
    }

    bool SerialPort::write(const std::vector<uint8_t>& data) {
        return sendCommand(std::string(data.begin(), data.end()));
    }

    bool SerialPort::write(const std::string& data) {
        return sendCommand(data);
    }

    std::vector<uint8_t> SerialPort::read(size_t maxBytes) {
        // This is a legacy method - not recommended for high performance
        std::vector<uint8_t> buffer;
        if (!m_isOpen || maxBytes == 0) {
            return buffer;
        }

#ifdef _WIN32
        buffer.resize(maxBytes);
        DWORD bytesRead = 0;
        bool result = ReadFile(m_handle, buffer.data(),
            static_cast<DWORD>(maxBytes), &bytesRead, nullptr);
        if (result && bytesRead > 0) {
            buffer.resize(bytesRead);
        }
        else {
            buffer.clear();
        }
#endif

        return buffer;
    }

    std::string SerialPort::readString(size_t maxBytes) {
        auto data = read(maxBytes);
        return std::string(data.begin(), data.end());
    }

    size_t SerialPort::available() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_isOpen) {
            return 0;
        }

#ifdef _WIN32
        COMSTAT comStat;
        DWORD errors;
        if (ClearCommError(m_handle, &errors, &comStat)) {
            return comStat.cbInQue;
        }
#endif

        return 0;
    }

    bool SerialPort::flush() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_isOpen) {
            return false;
        }

#ifdef _WIN32
        return FlushFileBuffers(m_handle) != 0;
#else
        return false;
#endif
    }

    void SerialPort::setTimeout(uint32_t timeoutMs) {
        m_timeout = timeoutMs;
        if (m_isOpen) {
            updateTimeouts();
        }
    }

    uint32_t SerialPort::getTimeout() const {
        return m_timeout;
    }

    std::vector<std::string> SerialPort::getAvailablePorts() {
        std::vector<std::string> ports;

#ifdef _WIN32
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            char valueName[256];
            char data[256];
            DWORD valueNameSize, dataSize, dataType;
            DWORD index = 0;

            while (true) {
                valueNameSize = sizeof(valueName);
                dataSize = sizeof(data);

                LONG result = RegEnumValueA(hKey, index++, valueName, &valueNameSize,
                    nullptr, &dataType,
                    reinterpret_cast<BYTE*>(data), &dataSize);

                if (result == ERROR_NO_MORE_ITEMS) {
                    break;
                }

                if (result == ERROR_SUCCESS && dataType == REG_SZ) {
                    ports.emplace_back(data);
                }
            }

            RegCloseKey(hKey);
        }
#endif

        std::sort(ports.begin(), ports.end());
        return ports;
    }

    std::vector<std::string> SerialPort::findMakcuPorts() {
        std::vector<std::string> makcuPorts;

#ifdef _WIN32
        auto allPorts = getAvailablePorts();
        HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS,
            nullptr, nullptr, DIGCF_PRESENT);
        if (deviceInfoSet == INVALID_HANDLE_VALUE) {
            return makcuPorts;
        }

        SP_DEVINFO_DATA deviceInfoData;
        deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); i++) {
            char description[256] = { 0 };
            char portName[256] = { 0 };

            if (SetupDiGetDeviceRegistryPropertyA(deviceInfoSet, &deviceInfoData,
                SPDRP_DEVICEDESC, nullptr,
                reinterpret_cast<BYTE*>(description),
                sizeof(description), nullptr)) {
                std::string desc(description);

                if (desc.find("USB-Enhanced-SERIAL CH343") != std::string::npos ||
                    desc.find("USB-SERIAL CH340") != std::string::npos) {

                    HKEY hDeviceKey = SetupDiOpenDevRegKey(deviceInfoSet, &deviceInfoData,
                        DICS_FLAG_GLOBAL, 0,
                        DIREG_DEV, KEY_READ);
                    if (hDeviceKey != INVALID_HANDLE_VALUE) {
                        DWORD portNameSize = sizeof(portName);

                        if (RegQueryValueExA(hDeviceKey, "PortName", nullptr, nullptr,
                            reinterpret_cast<BYTE*>(portName),
                            &portNameSize) == ERROR_SUCCESS) {
                            std::string port(portName);
                            if (std::find(allPorts.begin(), allPorts.end(), port) != allPorts.end()) {
                                makcuPorts.emplace_back(port);
                            }
                        }
                        RegCloseKey(hDeviceKey);
                    }
                }
            }
        }

        SetupDiDestroyDeviceInfoList(deviceInfoSet);
#endif

        std::sort(makcuPorts.begin(), makcuPorts.end());
        makcuPorts.erase(std::unique(makcuPorts.begin(), makcuPorts.end()), makcuPorts.end());
        return makcuPorts;
    }

} // namespace makcu