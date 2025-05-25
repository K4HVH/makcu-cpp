#include "../include/utilities.h"
#include <sstream>
#include <fstream>
#include <thread>
#include <random>
#include <algorithm>
#include <cstring>
#include <memory>
#include <atomic>
#include <cstdint>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#pragma comment(lib, "psapi.lib")
#endif

namespace makcu {

    // KeyAction implementation
    std::string KeyAction::serialize() const {
        std::ostringstream oss;
        oss << static_cast<int>(type) << "," << static_cast<int>(key) << "," << duration;
        return oss.str();
    }

    bool KeyAction::execute(Device& device) const {
        switch (type) {
        case ActionType::KEY_DOWN:
            return device.keyDown(key);
        case ActionType::KEY_UP:
            return device.keyUp(key);
        case ActionType::KEY_PRESS:
            return device.keyPress(key, duration);
        default:
            return false;
        }
    }

    // MultiKeyAction implementation
    std::string MultiKeyAction::serialize() const {
        std::ostringstream oss;
        oss << static_cast<int>(type) << "," << keys.size();
        for (auto key : keys) {
            oss << "," << static_cast<int>(key);
        }
        oss << "," << duration;
        return oss.str();
    }

    bool MultiKeyAction::execute(Device& device) const {
        return device.multiKeyPress(keys, duration);
    }

    // TypeStringAction implementation
    std::string TypeStringAction::serialize() const {
        std::ostringstream oss;
        oss << static_cast<int>(type) << "," << text.length() << "," << text;
        return oss.str();
    }

    bool TypeStringAction::execute(Device& device) const {
        return device.typeString(text);
    }

    // MouseButtonAction implementation
    std::string MouseButtonAction::serialize() const {
        std::ostringstream oss;
        oss << static_cast<int>(type) << "," << static_cast<int>(button) << "," << count;
        return oss.str();
    }

    bool MouseButtonAction::execute(Device& device) const {
        switch (type) {
        case ActionType::MOUSE_DOWN:
            return device.mouseDown(button);
        case ActionType::MOUSE_UP:
            return device.mouseUp(button);
        case ActionType::MOUSE_CLICK:
            return device.mouseClick(button, count);
        default:
            return false;
        }
    }

    // MouseMoveAction implementation
    std::string MouseMoveAction::serialize() const {
        std::ostringstream oss;
        oss << static_cast<int>(type) << "," << x << "," << y;
        return oss.str();
    }

    bool MouseMoveAction::execute(Device& device) const {
        if (absolute) {
            return device.mouseMoveTo(x, y);
        } else {
            return device.mouseMove(x, y);
        }
    }

    // MouseWheelAction implementation
    std::string MouseWheelAction::serialize() const {
        std::ostringstream oss;
        oss << static_cast<int>(type) << "," << delta;
        return oss.str();
    }

    bool MouseWheelAction::execute(Device& device) const {
        return device.mouseWheel(delta);
    }

    // DelayAction implementation
    std::string DelayAction::serialize() const {
        std::ostringstream oss;
        oss << static_cast<int>(type) << "," << milliseconds;
        return oss.str();
    }

    bool DelayAction::execute(Device& device) const {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        return true;
    }

    // MacroRecorder implementation
    MacroRecorder::MacroRecorder()
        : m_recording(false)
        , m_playing(false)
        , m_recordMouseMovement(false)
        , m_minimumDelay(10)
        , m_useTimestamps(true)
        , m_stopPlayback(false)
    {
    }

    MacroRecorder::~MacroRecorder() {
        stopRecording();
        stopPlayback();
    }

    bool MacroRecorder::startRecording() {
        if (m_recording) {
            return false;
        }

        m_actions.clear();
        m_recordingStart = std::chrono::steady_clock::now();
        m_recording = true;
        return true;
    }

    bool MacroRecorder::stopRecording() {
        if (!m_recording) {
            return false;
        }

        m_recording = false;
        return true;
    }

    bool MacroRecorder::isRecording() const {
        return m_recording;
    }

    bool MacroRecorder::playback(Device& device, int repeatCount) {
        if (m_playing || m_actions.empty()) {
            return false;
        }

        m_playing = true;
        m_stopPlayback = false;

        for (int rep = 0; rep < repeatCount && !m_stopPlayback; ++rep) {
            auto previousTimestamp = std::chrono::milliseconds(0);

            for (const auto& action : m_actions) {
                if (m_stopPlayback) break;

                // Handle timing
                if (m_useTimestamps && action->timestamp > previousTimestamp) {
                    auto delay = action->timestamp - previousTimestamp;
                    if (delay >= std::chrono::milliseconds(m_minimumDelay)) {
                        std::this_thread::sleep_for(delay);
                    }
                }

                // Execute action
                action->execute(device);
                previousTimestamp = action->timestamp;
            }
        }

        m_playing = false;
        return true;
    }

    bool MacroRecorder::playbackAsync(Device& device, int repeatCount) {
        if (m_playing) {
            return false;
        }

        m_playbackThread = std::thread(&MacroRecorder::playbackLoop, this, std::ref(device), repeatCount);
        return true;
    }

    void MacroRecorder::stopPlayback() {
        m_stopPlayback = true;
        if (m_playbackThread.joinable()) {
            m_playbackThread.join();
        }
        m_playing = false;
    }

    void MacroRecorder::clear() {
        if (m_recording || m_playing) {
            return;
        }
        m_actions.clear();
    }

    size_t MacroRecorder::getActionCount() const {
        return m_actions.size();
    }

    std::chrono::milliseconds MacroRecorder::getTotalDuration() const {
        if (m_actions.empty()) {
            return std::chrono::milliseconds(0);
        }
        return m_actions.back()->timestamp;
    }

    bool MacroRecorder::saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        file << "MAKCU_MACRO_V1\n";
        file << m_actions.size() << "\n";

        for (const auto& action : m_actions) {
            file << action->timestamp.count() << "," << action->serialize() << "\n";
        }

        return true;
    }

    bool MacroRecorder::loadFromFile(const std::string& filename) {
        if (m_recording || m_playing) {
            return false;
        }

        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        std::string header;
        std::getline(file, header);
        if (header != "MAKCU_MACRO_V1") {
            return false;
        }

        size_t actionCount;
        file >> actionCount;
        file.ignore(); // Skip newline

        m_actions.clear();
        m_actions.reserve(actionCount);

        // Note: Full implementation would parse the serialized actions
        // This is a simplified version showing the structure
        for (size_t i = 0; i < actionCount; ++i) {
            std::string line;
            std::getline(file, line);
            // Parse line and create appropriate action
            // Implementation depends on the specific action serialization format
        }

        return true;
    }

    void MacroRecorder::addKeyPress(KeyCode key, uint32_t duration) {
        auto action = std::make_unique<KeyAction>(ActionType::KEY_PRESS, key, duration);
        action->timestamp = getCurrentTimestamp();
        m_actions.push_back(std::move(action));
    }

    void MacroRecorder::addMultiKeyPress(const std::vector<KeyCode>& keys, uint32_t duration) {
        auto action = std::make_unique<MultiKeyAction>(keys, duration);
        action->timestamp = getCurrentTimestamp();
        m_actions.push_back(std::move(action));
    }

    void MacroRecorder::addTypeString(const std::string& text) {
        auto action = std::make_unique<TypeStringAction>(text);
        action->timestamp = getCurrentTimestamp();
        m_actions.push_back(std::move(action));
    }

    void MacroRecorder::addMouseClick(MouseButton button, uint32_t count) {
        auto action = std::make_unique<MouseButtonAction>(ActionType::MOUSE_CLICK, button, count);
        action->timestamp = getCurrentTimestamp();
        m_actions.push_back(std::move(action));
    }

    void MacroRecorder::addMouseMove(int32_t x, int32_t y, bool absolute) {
        auto action = std::make_unique<MouseMoveAction>(x, y, absolute);
        action->timestamp = getCurrentTimestamp();
        m_actions.push_back(std::move(action));
    }

    void MacroRecorder::addMouseWheel(int32_t delta) {
        auto action = std::make_unique<MouseWheelAction>(delta);
        action->timestamp = getCurrentTimestamp();
        m_actions.push_back(std::move(action));
    }

    void MacroRecorder::addDelay(uint32_t milliseconds) {
        auto action = std::make_unique<DelayAction>(milliseconds);
        action->timestamp = getCurrentTimestamp();
        m_actions.push_back(std::move(action));
    }

    void MacroRecorder::onKeyboard(KeyCode key, bool isPressed) {
        if (!m_recording) return;

        ActionType type = isPressed ? ActionType::KEY_DOWN : ActionType::KEY_UP;
        auto action = std::make_unique<KeyAction>(type, key);
        action->timestamp = getCurrentTimestamp();
        m_actions.push_back(std::move(action));
    }

    void MacroRecorder::onMouseButton(MouseButton button, bool isPressed) {
        if (!m_recording) return;

        ActionType type = isPressed ? ActionType::MOUSE_DOWN : ActionType::MOUSE_UP;
        auto action = std::make_unique<MouseButtonAction>(type, button);
        action->timestamp = getCurrentTimestamp();
        m_actions.push_back(std::move(action));
    }

    void MacroRecorder::onMouseMove(int32_t x, int32_t y) {
        if (!m_recording || !m_recordMouseMovement) return;

        auto action = std::make_unique<MouseMoveAction>(x, y, false);
        action->timestamp = getCurrentTimestamp();
        m_actions.push_back(std::move(action));
    }

    void MacroRecorder::onMouseWheel(int32_t delta) {
        if (!m_recording) return;

        auto action = std::make_unique<MouseWheelAction>(delta);
        action->timestamp = getCurrentTimestamp();
        m_actions.push_back(std::move(action));
    }

    void MacroRecorder::setRecordMouseMovement(bool record) {
        m_recordMouseMovement = record;
    }

    void MacroRecorder::setMinimumDelay(uint32_t milliseconds) {
        m_minimumDelay = milliseconds;
    }

    void MacroRecorder::setTimestampMode(bool useTimestamps) {
        m_useTimestamps = useTimestamps;
    }

    void MacroRecorder::playbackLoop(Device& device, int repeatCount) {
        playback(device, repeatCount);
    }

    std::chrono::milliseconds MacroRecorder::getCurrentTimestamp() const {
        if (!m_recording) {
            return std::chrono::milliseconds(0);
        }

        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_recordingStart);
    }

    // AutomationHelper implementation
    bool AutomationHelper::waitForWindow(const std::string& windowTitle, uint32_t timeoutMs) {
#ifdef _WIN32
        auto start = std::chrono::steady_clock::now();
        
        while (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(timeoutMs)) {
            HWND hwnd = FindWindowA(NULL, windowTitle.c_str());
            if (hwnd != NULL) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
#endif
        return false;
    }

    bool AutomationHelper::waitForPixelColor(int32_t x, int32_t y, uint32_t color, uint32_t timeoutMs) {
        auto start = std::chrono::steady_clock::now();
        
        while (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(timeoutMs)) {
            if (getPixelColor(x, y) == color) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        return false;
    }

    uint32_t AutomationHelper::getPixelColor(int32_t x, int32_t y) {
#ifdef _WIN32
        HDC hdc = GetDC(NULL);
        COLORREF color = GetPixel(hdc, x, y);
        ReleaseDC(NULL, hdc);
        return static_cast<uint32_t>(color);
#else
        return 0;
#endif
    }

    std::string AutomationHelper::getClipboardText() {
#ifdef _WIN32
        if (!OpenClipboard(NULL)) {
            return "";
        }

        HANDLE hData = GetClipboardData(CF_TEXT);
        if (hData == NULL) {
            CloseClipboard();
            return "";
        }

        char* pszText = static_cast<char*>(GlobalLock(hData));
        if (pszText == NULL) {
            CloseClipboard();
            return "";
        }

        std::string text(pszText);
        GlobalUnlock(hData);
        CloseClipboard();
        return text;
#else
        return "";
#endif
    }

    bool AutomationHelper::setClipboardText(const std::string& text) {
#ifdef _WIN32
        if (!OpenClipboard(NULL)) {
            return false;
        }

        EmptyClipboard();

        HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, text.size() + 1);
        if (hClipboardData == NULL) {
            CloseClipboard();
            return false;
        }

        char* pchData = static_cast<char*>(GlobalLock(hClipboardData));
        if (pchData == NULL) {
            GlobalFree(hClipboardData);
            CloseClipboard();
            return false;
        }

#ifdef _WIN32
        strcpy_s(pchData, text.size() + 1, text.c_str());
#else
        strcpy(pchData, text.c_str());
#endif
        GlobalUnlock(hClipboardData);

        SetClipboardData(CF_TEXT, hClipboardData);
        CloseClipboard();
        return true;
#else
        return false;
#endif
    }

    std::vector<std::string> AutomationHelper::getRunningProcesses() {
        std::vector<std::string> processes;

#ifdef _WIN32
        HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hProcessSnap == INVALID_HANDLE_VALUE) {
            return processes;
        }

        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hProcessSnap, &pe32)) {
            do {
                processes.emplace_back(pe32.szExeFile);
            } while (Process32Next(hProcessSnap, &pe32));
        }

        CloseHandle(hProcessSnap);
#endif

        return processes;
    }

    bool AutomationHelper::isProcessRunning(const std::string& processName) {
        auto processes = getRunningProcesses();
        return std::find(processes.begin(), processes.end(), processName) != processes.end();
    }

    // GamingUtilities implementation
    bool GamingUtilities::performRecoilControl(Device& device, 
                                              const std::vector<std::pair<int32_t, int32_t>>& pattern,
                                              uint32_t intervalMs) {
        for (const auto& movement : pattern) {
            device.mouseMove(movement.first, movement.second);
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }
        return true;
    }

    bool GamingUtilities::performClickSequence(Device& device,
                                              const std::vector<MouseButton>& buttons,
                                              uint32_t intervalMs) {
        for (auto button : buttons) {
            device.mouseClick(button);
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }
        return true;
    }

    bool GamingUtilities::performKeySequence(Device& device,
                                            const std::vector<KeyCode>& keys,
                                            uint32_t intervalMs) {
        for (auto key : keys) {
            device.keyPress(key);
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }
        return true;
    }

    bool GamingUtilities::autoClick(Device& device, MouseButton button, 
                                   uint32_t intervalMs, uint32_t durationMs) {
        auto start = std::chrono::steady_clock::now();
        auto end = start + std::chrono::milliseconds(durationMs);

        while (std::chrono::steady_clock::now() < end) {
            device.mouseClick(button);
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }

        return true;
    }

    uint32_t GamingUtilities::humanizeDelay(uint32_t baseDelay, uint32_t variance) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        
        std::uniform_int_distribution<> dis(-static_cast<int>(variance), static_cast<int>(variance));
        int32_t randomOffset = dis(gen);
        
        return std::max(1u, static_cast<uint32_t>(static_cast<int32_t>(baseDelay) + randomOffset));
    }

    std::pair<int32_t, int32_t> GamingUtilities::humanizeMovement(int32_t x, int32_t y, uint32_t variance) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        
        std::uniform_int_distribution<> dis(-static_cast<int>(variance), static_cast<int>(variance));
        
        return {x + dis(gen), y + dis(gen)};
    }

} // namespace makcu
