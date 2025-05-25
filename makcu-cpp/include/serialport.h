#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#endif

namespace makcu {

    class SerialPort {
    public:
        SerialPort();
        ~SerialPort();

        bool open(const std::string& port, uint32_t baudRate);
        void close();
        bool isOpen() const;

        bool setBaudRate(uint32_t baudRate);
        uint32_t getBaudRate() const;

        std::string getPortName() const;

        // Data transmission
        bool write(const std::vector<uint8_t>& data);
        bool write(const std::string& data);
        std::vector<uint8_t> read(size_t maxBytes = 1024);
        std::string readString(size_t maxBytes = 1024);
        
        bool writeByte(uint8_t byte);
        bool readByte(uint8_t& byte);
        
        size_t available() const;
        bool flush();

        // Timeout control
        void setTimeout(uint32_t timeoutMs);
        uint32_t getTimeout() const;

        // Port enumeration
        static std::vector<std::string> getAvailablePorts();
        static std::vector<std::string> findMakcuPorts();

    private:
        std::string m_portName;
        uint32_t m_baudRate;
        uint32_t m_timeout;
        std::atomic<bool> m_isOpen;
        mutable std::mutex m_mutex;

#ifdef _WIN32
        HANDLE m_handle;
        DCB m_dcb;
        COMMTIMEOUTS m_timeouts;
#else
        int m_fd;
#endif

        bool configurePort();
        void updateTimeouts();

        // Disable copy constructor and assignment
        SerialPort(const SerialPort&) = delete;
        SerialPort& operator=(const SerialPort&) = delete;
    };

} // namespace makcu
