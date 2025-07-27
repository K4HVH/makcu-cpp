#pragma once

#include <string>
#include <chrono>
#include <random>
#include <tuple>
#include <unordered_map>
#include <functional>
#include <vector>
#include "makcu.h"

namespace makcu {

    class HumanPatterns {
    public:
        enum class MovementCurve {
            LINEAR,
            EASE_IN,
            EASE_OUT,
            EASE_IN_OUT,
            BEZIER_NATURAL,
            OVERSHOOT
        };

        enum class ClickVariation {
            CONSISTENT,
            SLIGHT_VARIATION,
            MODERATE_VARIATION,
            HIGH_VARIATION,
            CHAOTIC
        };

        struct CustomProfile {
            // Timing ranges
            std::chrono::milliseconds press_min;
            std::chrono::milliseconds press_max;
            std::chrono::milliseconds release_min;
            std::chrono::milliseconds release_max;
            
            // Jitter configuration
            int32_t jitter_radius = 0;
            double jitter_probability = 0.0;
            std::chrono::milliseconds jitter_delay_min = std::chrono::milliseconds(5);
            std::chrono::milliseconds jitter_delay_max = std::chrono::milliseconds(15);
            
            // Movement characteristics
            MovementCurve movement_curve = MovementCurve::LINEAR;
            uint32_t movement_segments = 10;
            double smoothness_factor = 1.0;
            
            // Click variation pattern
            ClickVariation click_variation = ClickVariation::SLIGHT_VARIATION;
            double timing_drift = 0.1;
            
            // Fatigue simulation
            bool enable_fatigue = false;
            double fatigue_factor = 0.0;
            uint32_t fatigue_threshold = 50;
            
            // Acceleration patterns
            bool use_acceleration = false;
            double acceleration_factor = 1.0;
            uint32_t acceleration_buildup = 5;
        };

        enum class Profile {
            PRECISION,
            CASUAL,
            GAMING,
            SLEEPY,
            CAFFEINATED,
            NERVOUS,
            FOCUSED,
            DISTRACTED
        };

        explicit HumanPatterns(Device* device);

        bool clickHumanLike(MouseButton button, int count = 1, 
                           Profile profile = Profile::CASUAL);
        
        bool moveHumanLike(int32_t x, int32_t y, Profile profile = Profile::CASUAL);
        
        bool clickSequenceHumanLike(const std::vector<MouseButton>& buttons,
                                   Profile profile = Profile::CASUAL);

        bool dragHumanLike(int32_t start_x, int32_t start_y, 
                          int32_t end_x, int32_t end_y, 
                          MouseButton button = MouseButton::LEFT,
                          Profile profile = Profile::CASUAL);

        bool scrollHumanLike(int32_t delta, int scroll_steps = 3,
                            Profile profile = Profile::CASUAL);

        void setCustomProfile(const std::string& name, const CustomProfile& profile);
        bool setActiveProfile(const std::string& name);
        void resetToDefaultProfile(Profile profile);

        void setSessionFatigue(double fatigue_level);

        static std::string profileToString(Profile profile);
        static Profile stringToProfile(const std::string& profileName);

    private:
        Device* m_device;
        std::mt19937 m_rng;
        std::uniform_real_distribution<double> m_distribution;
        std::normal_distribution<double> m_normal_distribution;
        
        static const std::unordered_map<Profile, CustomProfile> s_defaultProfiles;
        std::unordered_map<std::string, CustomProfile> m_customProfiles;
        std::string m_activeCustomProfile;
        
        uint32_t m_actionCount;
        double m_currentFatigue;
        
        CustomProfile getCurrentProfile(Profile profile) const;
        std::chrono::milliseconds getRandomTiming(std::chrono::milliseconds min, 
                                                 std::chrono::milliseconds max,
                                                 const CustomProfile& profile);
        void applyJitter(const CustomProfile& profile);
        void performClick(MouseButton button, const CustomProfile& profile);
        void performMovement(int32_t x, int32_t y, const CustomProfile& profile);
        
        double calculateCurvePoint(double t, MovementCurve curve) const;
        std::chrono::milliseconds applyVariation(std::chrono::milliseconds base, 
                                                const CustomProfile& profile);
        void updateFatigue();
        
        std::pair<int32_t, int32_t> generateJitterOffset(const CustomProfile& profile);
        std::vector<std::pair<int32_t, int32_t>> generateMovementPath(
            int32_t start_x, int32_t start_y, int32_t end_x, int32_t end_y, 
            const CustomProfile& profile);
    };

} // namespace makcu