#pragma once

#include "makcu.h"
#include <vector>
#include <chrono>
#include <fstream>
#include <functional>

namespace makcu {

    // Macro action types
    enum class ActionType {
        KEY_DOWN,
        KEY_UP,
        KEY_PRESS,
        MULTI_KEY_PRESS,
        TYPE_STRING,
        MOUSE_DOWN,
        MOUSE_UP,
        MOUSE_CLICK,
        MOUSE_MOVE,
        MOUSE_MOVETO,
        MOUSE_WHEEL,
        DELAY
    };

    // Base action class
    struct Action {
        ActionType type;
        std::chrono::milliseconds timestamp;
        
        Action(ActionType t) : type(t), timestamp(std::chrono::milliseconds(0)) {}
        virtual ~Action() = default;
        virtual std::string serialize() const = 0;
        virtual bool execute(Device& device) const = 0;
    };

    // Specific action implementations
    struct KeyAction : public Action {
        KeyCode key;
        uint32_t duration;
        
        KeyAction(ActionType t, KeyCode k, uint32_t d = 0) 
            : Action(t), key(k), duration(d) {}
        
        std::string serialize() const override;
        bool execute(Device& device) const override;
    };

    struct MultiKeyAction : public Action {
        std::vector<KeyCode> keys;
        uint32_t duration;
        
        MultiKeyAction(const std::vector<KeyCode>& k, uint32_t d = 0)
            : Action(ActionType::MULTI_KEY_PRESS), keys(k), duration(d) {}
        
        std::string serialize() const override;
        bool execute(Device& device) const override;
    };

    struct TypeStringAction : public Action {
        std::string text;
        
        TypeStringAction(const std::string& t) 
            : Action(ActionType::TYPE_STRING), text(t) {}
        
        std::string serialize() const override;
        bool execute(Device& device) const override;
    };

    struct MouseButtonAction : public Action {
        MouseButton button;
        uint32_t count;
        
        MouseButtonAction(ActionType t, MouseButton b, uint32_t c = 1)
            : Action(t), button(b), count(c) {}
        
        std::string serialize() const override;
        bool execute(Device& device) const override;
    };

    struct MouseMoveAction : public Action {
        int32_t x, y;
        bool absolute;
        
        MouseMoveAction(int32_t x_, int32_t y_, bool abs = false)
            : Action(abs ? ActionType::MOUSE_MOVETO : ActionType::MOUSE_MOVE)
            , x(x_), y(y_), absolute(abs) {}
        
        std::string serialize() const override;
        bool execute(Device& device) const override;
    };

    struct MouseWheelAction : public Action {
        int32_t delta;
        
        MouseWheelAction(int32_t d) : Action(ActionType::MOUSE_WHEEL), delta(d) {}
        
        std::string serialize() const override;
        bool execute(Device& device) const override;
    };

    struct DelayAction : public Action {
        uint32_t milliseconds;
        
        DelayAction(uint32_t ms) : Action(ActionType::DELAY), milliseconds(ms) {}
        
        std::string serialize() const override;
        bool execute(Device& device) const override;
    };

    // Macro recorder class
    class MacroRecorder {
    public:
        MacroRecorder();
        ~MacroRecorder();

        // Recording control
        bool startRecording();
        bool stopRecording();
        bool isRecording() const;

        // Playback control
        bool playback(Device& device, int repeatCount = 1);
        bool playbackAsync(Device& device, int repeatCount = 1);
        void stopPlayback();

        // Macro management
        void clear();
        size_t getActionCount() const;
        std::chrono::milliseconds getTotalDuration() const;

        // File operations
        bool saveToFile(const std::string& filename) const;
        bool loadFromFile(const std::string& filename);

        // Manual action addition
        void addKeyPress(KeyCode key, uint32_t duration = 0);
        void addMultiKeyPress(const std::vector<KeyCode>& keys, uint32_t duration = 0);
        void addTypeString(const std::string& text);
        void addMouseClick(MouseButton button, uint32_t count = 1);
        void addMouseMove(int32_t x, int32_t y, bool absolute = false);
        void addMouseWheel(int32_t delta);
        void addDelay(uint32_t milliseconds);

        // Event callbacks for recording
        void onKeyboard(KeyCode key, bool isPressed);
        void onMouseButton(MouseButton button, bool isPressed);
        void onMouseMove(int32_t x, int32_t y);
        void onMouseWheel(int32_t delta);

        // Settings
        void setRecordMouseMovement(bool record);
        void setMinimumDelay(uint32_t milliseconds);
        void setTimestampMode(bool useTimestamps);

    private:
        std::vector<std::unique_ptr<Action>> m_actions;
        std::chrono::steady_clock::time_point m_recordingStart;
        bool m_recording;
        bool m_playing;
        bool m_recordMouseMovement;
        uint32_t m_minimumDelay;
        bool m_useTimestamps;
        
        std::thread m_playbackThread;
        std::atomic<bool> m_stopPlayback;

        void playbackLoop(Device& device, int repeatCount);
        std::chrono::milliseconds getCurrentTimestamp() const;
    };

    // Automation helper class
    class AutomationHelper {
    public:
        static bool waitForWindow(const std::string& windowTitle, uint32_t timeoutMs = 5000);
        static bool waitForPixelColor(int32_t x, int32_t y, uint32_t color, uint32_t timeoutMs = 5000);
        static bool waitForKeyPress(KeyCode key, uint32_t timeoutMs = 5000);
        static bool waitForMouseButton(MouseButton button, uint32_t timeoutMs = 5000);

        // Screen capture utilities
        static bool captureScreen(const std::string& filename);
        static bool captureRegion(int32_t x, int32_t y, int32_t width, int32_t height, const std::string& filename);
        static uint32_t getPixelColor(int32_t x, int32_t y);

        // Pattern recognition
        static bool findImageOnScreen(const std::string& imageFile, int32_t& x, int32_t& y, double threshold = 0.8);
        static bool findColorOnScreen(uint32_t color, int32_t& x, int32_t& y, int32_t tolerance = 0);

        // Text utilities
        static std::string getClipboardText();
        static bool setClipboardText(const std::string& text);

        // System utilities
        static std::vector<std::string> getRunningProcesses();
        static bool isProcessRunning(const std::string& processName);
        static bool killProcess(const std::string& processName);
        static bool startProcess(const std::string& path, const std::string& args = "");
    };

    // Gaming-specific utilities
    class GamingUtilities {
    public:
        // Common gaming patterns
        static bool performRecoilControl(Device& device, 
                                       const std::vector<std::pair<int32_t, int32_t>>& pattern,
                                       uint32_t intervalMs = 10);
        
        static bool performClickSequence(Device& device,
                                       const std::vector<MouseButton>& buttons,
                                       uint32_t intervalMs = 100);

        static bool performKeySequence(Device& device,
                                     const std::vector<KeyCode>& keys,
                                     uint32_t intervalMs = 100);

        // Auto-clicker functionality
        static bool autoClick(Device& device, MouseButton button, 
                            uint32_t intervalMs, uint32_t durationMs);

        // Spam prevention (humanization)
        static uint32_t humanizeDelay(uint32_t baseDelay, uint32_t variance = 20);
        static std::pair<int32_t, int32_t> humanizeMovement(int32_t x, int32_t y, uint32_t variance = 2);

        // Hotkey management
        class HotkeyManager {
        public:
            using HotkeyCallback = std::function<void()>;
            
            bool registerHotkey(const std::vector<KeyCode>& keys, HotkeyCallback callback);
            bool unregisterHotkey(const std::vector<KeyCode>& keys);
            void unregisterAll();
            
            void startMonitoring(Device& device);
            void stopMonitoring();

        private:
            struct Hotkey {
                std::vector<KeyCode> keys;
                HotkeyCallback callback;
                bool active;
            };

            std::vector<Hotkey> m_hotkeys;
            std::thread m_monitorThread;
            std::atomic<bool> m_monitoring;
            Device* m_device;

            void monitoringLoop();
            bool checkHotkeyPressed(const Hotkey& hotkey);
        };
    };

} // namespace makcu
