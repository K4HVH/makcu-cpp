#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <future>
#include <unordered_map>
#include <thread>
#include <queue>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

namespace makcu {

    struct PendingCommand {
        int command_id;
        std::string command;
        std::promise<std::string> promise;
        std::chrono::steady_clock::time_point timestamp;
        bool expect_response;
        std::chrono::milliseconds timeout;

        PendingCommand(int id, const std::string& cmd, bool expect_resp, std::chrono::milliseconds to)
            : command_id(id), command(cmd), expect_response(expect_resp), timeout(to) {
            timestamp = std::chrono::steady_clock::now();
        }
    };

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

        // High-performance command execution with tracking
        std::future<std::string> sendTrackedCommand(const std::string& command,
            bool expectResponse = false,
            std::chrono::milliseconds timeout = std::chrono::milliseconds(100));

        // Fast fire-and-forget commands
        bool sendCommand(const std::string& command);

        // Legacy methods for compatibility
        bool write(const std::vector<uint8_t>& data);
        bool write(const std::string& data);
        std::vector<uint8_t> read(size_t maxBytes = 1024);
        std::string readString(size_t maxBytes = 1024);

        size_t available() const;
        bool flush();

        // Optimized timeout control
        void setTimeout(uint32_t timeoutMs);
        uint32_t getTimeout() const;

        // Port enumeration
        static std::vector<std::string> getAvailablePorts();
        static std::vector<std::string> findMakcuPorts();

        // Button callback support
        using ButtonCallback = std::function<void(uint8_t, bool)>;
        void setButtonCallback(ButtonCallback callback);

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

        // Command tracking system
        std::atomic<int> m_commandCounter{ 0 };
        std::unordered_map<int, std::unique_ptr<PendingCommand>> m_pendingCommands;
        std::mutex m_commandMutex;

        // High-performance listener thread
        std::thread m_listenerThread;
        std::atomic<bool> m_stopListener{ false };

        // Button data processing
        ButtonCallback m_buttonCallback;
        std::atomic<uint8_t> m_lastButtonMask{ 0 };

        // Optimized parsing buffers
        static constexpr size_t BUFFER_SIZE = 4096;
        static constexpr size_t LINE_BUFFER_SIZE = 256;

        bool configurePort();
        void updateTimeouts();
        void listenerLoop();
        void processIncomingData();
        void handleButtonData(uint8_t data);
        void processResponse(const std::string& response);
        void cleanupTimedOutCommands();
        int generateCommandId();

        // Disable copy
        SerialPort(const SerialPort&) = delete;
        SerialPort& operator=(const SerialPort&) = delete;
    };

} // namespace makcu