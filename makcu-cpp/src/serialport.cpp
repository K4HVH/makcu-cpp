#include "../include/serialport.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>

#ifdef _WIN32
#include <setupapi.h>
#include <devguid.h>
#include <cfgmgr32.h>
#pragma comment(lib, "setupapi.lib")
#endif

namespace makcu {

    SerialPort::SerialPort() 
        : m_baudRate(115200)
        , m_timeout(1000)
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
        return true;
#else
        // Linux implementation would go here
        return false;
#endif
    }

    void SerialPort::close() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isOpen) {
            return;
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
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isOpen || data.empty()) {
            return false;
        }

#ifdef _WIN32
        DWORD bytesWritten = 0;
        BOOL result = WriteFile(
            m_handle,
            data.data(),
            static_cast<DWORD>(data.size()),
            &bytesWritten,
            nullptr
        );

        return result && (bytesWritten == data.size());
#else
        return false;
#endif
    }

    bool SerialPort::write(const std::string& data) {
        std::vector<uint8_t> bytes(data.begin(), data.end());
        return write(bytes);
    }

    std::vector<uint8_t> SerialPort::read(size_t maxBytes) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::vector<uint8_t> buffer;
        if (!m_isOpen || maxBytes == 0) {
            return buffer;
        }

#ifdef _WIN32
        buffer.resize(maxBytes);
        DWORD bytesRead = 0;
        
        BOOL result = ReadFile(
            m_handle,
            buffer.data(),
            static_cast<DWORD>(maxBytes),
            &bytesRead,
            nullptr
        );

        if (result && bytesRead > 0) {
            buffer.resize(bytesRead);
        } else {
            buffer.clear();
        }
#endif

        return buffer;
    }

    std::string SerialPort::readString(size_t maxBytes) {
        auto data = read(maxBytes);
        return std::string(data.begin(), data.end());
    }

    bool SerialPort::writeByte(uint8_t byte) {
        return write(std::vector<uint8_t>{byte});
    }

    bool SerialPort::readByte(uint8_t& byte) {
        auto data = read(1);
        if (!data.empty()) {
            byte = data[0];
            return true;
        }
        return false;
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
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            char valueName[256];
            char data[256];
            DWORD valueNameSize, dataSize, dataType;
            DWORD index = 0;

            while (true) {
                valueNameSize = sizeof(valueName);
                dataSize = sizeof(data);

                LONG result = RegEnumValueA(hKey, index++, valueName, &valueNameSize, nullptr, &dataType, 
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
        auto allPorts = getAvailablePorts();

#ifdef _WIN32
        HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, nullptr, nullptr, DIGCF_PRESENT);
        if (deviceInfoSet == INVALID_HANDLE_VALUE) {
            return makcuPorts;
        }

        SP_DEVINFO_DATA deviceInfoData;
        deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        
        for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); i++) {
            char buffer[256];
            DWORD bufferSize = sizeof(buffer);
            
            if (SetupDiGetDeviceRegistryPropertyA(deviceInfoSet, &deviceInfoData, SPDRP_DEVICEDESC, 
                                                 nullptr, reinterpret_cast<BYTE*>(buffer), bufferSize, nullptr)) {
                std::string description(buffer);
                
                if (description.find("USB-Enhanced-SERIAL CH343") != std::string::npos ||
                    description.find("USB-SERIAL CH340") != std::string::npos) {
                    
                    // Get the COM port name
                    HKEY hDeviceKey = SetupDiOpenDevRegKey(deviceInfoSet, &deviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
                    if (hDeviceKey != INVALID_HANDLE_VALUE) {
                        char portName[256];
                        DWORD portNameSize = sizeof(portName);
                        
                        if (RegQueryValueExA(hDeviceKey, "PortName", nullptr, nullptr, 
                                           reinterpret_cast<BYTE*>(portName), &portNameSize) == ERROR_SUCCESS) {
                            makcuPorts.emplace_back(portName);
                        }
                        
                        RegCloseKey(hDeviceKey);
                    }
                }
            }
        }

        SetupDiDestroyDeviceInfoList(deviceInfoSet);
#endif

        return makcuPorts;
    }

    bool SerialPort::configurePort() {
        if (!m_isOpen) {
            return false;
        }

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
        if (!m_isOpen) {
            return;
        }

#ifdef _WIN32
        m_timeouts.ReadIntervalTimeout = 50;
        m_timeouts.ReadTotalTimeoutConstant = m_timeout;
        m_timeouts.ReadTotalTimeoutMultiplier = 10;
        m_timeouts.WriteTotalTimeoutConstant = m_timeout;
        m_timeouts.WriteTotalTimeoutMultiplier = 10;

        SetCommTimeouts(m_handle, &m_timeouts);
#endif
    }

} // namespace makcu
