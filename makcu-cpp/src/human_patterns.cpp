#include "../include/human_patterns.h"
#include <thread>
#include <algorithm>
#include <cmath>

namespace makcu {

    const std::unordered_map<HumanPatterns::Profile, HumanPatterns::CustomProfile> 
        HumanPatterns::s_defaultProfiles = {
        {Profile::PRECISION, {
            std::chrono::milliseconds(80), std::chrono::milliseconds(120),
            std::chrono::milliseconds(80), std::chrono::milliseconds(150),
            1, 0.1, std::chrono::milliseconds(10), std::chrono::milliseconds(25),
            MovementCurve::EASE_IN_OUT, 15, 0.8,
            ClickVariation::CONSISTENT, 0.05,
            false, 0.0, 100,
            false, 1.0, 10
        }},
        {Profile::CASUAL, {
            std::chrono::milliseconds(90), std::chrono::milliseconds(180),
            std::chrono::milliseconds(120), std::chrono::milliseconds(250),
            5, 0.6, std::chrono::milliseconds(5), std::chrono::milliseconds(20),
            MovementCurve::BEZIER_NATURAL, 12, 1.0,
            ClickVariation::SLIGHT_VARIATION, 0.15,
            true, 0.1, 60,
            true, 1.1, 8
        }},
        {Profile::GAMING, {
            std::chrono::milliseconds(15), std::chrono::milliseconds(35),
            std::chrono::milliseconds(25), std::chrono::milliseconds(55),
            3, 0.2, std::chrono::milliseconds(3), std::chrono::milliseconds(8),
            MovementCurve::LINEAR, 8, 1.2,
            ClickVariation::MODERATE_VARIATION, 0.08,
            false, 0.0, 200,
            true, 1.3, 3
        }},
        {Profile::SLEEPY, {
            std::chrono::milliseconds(150), std::chrono::milliseconds(300),
            std::chrono::milliseconds(200), std::chrono::milliseconds(400),
            12, 0.9, std::chrono::milliseconds(15), std::chrono::milliseconds(40),
            MovementCurve::EASE_OUT, 20, 0.6,
            ClickVariation::HIGH_VARIATION, 0.3,
            true, 0.4, 30,
            false, 1.0, 15
        }},
        {Profile::CAFFEINATED, {
            std::chrono::milliseconds(25), std::chrono::milliseconds(60),
            std::chrono::milliseconds(30), std::chrono::milliseconds(80),
            15, 0.7, std::chrono::milliseconds(2), std::chrono::milliseconds(10),
            MovementCurve::OVERSHOOT, 6, 1.4,
            ClickVariation::CHAOTIC, 0.2,
            false, 0.0, 150,
            true, 1.6, 2
        }},
    };

    HumanPatterns::HumanPatterns(Device* device)
        : m_device(device)
        , m_rng(std::random_device{}())
        , m_distribution(0.0, 1.0)
        , m_normal_distribution(0.0, 1.0)
        , m_actionCount(0)
        , m_currentFatigue(0.0)
 {
    }

    bool HumanPatterns::clickHumanLike(MouseButton button, int count, Profile profile) {
        if (!m_device || !m_device->isConnected()) {
            return false;
        }

        CustomProfile customProfile = getCurrentProfile(profile);
        
        for (int i = 0; i < count; ++i) {
            if (customProfile.jitter_probability > 0 && 
                m_distribution(m_rng) < customProfile.jitter_probability) {
                applyJitter(customProfile);
            }
            
            performClick(button, customProfile);
            updateFatigue();
            
            if (i < count - 1) {
                auto interClickDelay = getRandomTiming(customProfile.release_min, 
                                                      customProfile.release_max, customProfile);
                std::this_thread::sleep_for(interClickDelay);
            }
        }
        
        return true;
    }

    bool HumanPatterns::moveHumanLike(int32_t x, int32_t y, Profile profile) {
        if (!m_device || !m_device->isConnected()) {
            return false;
        }

        CustomProfile customProfile = getCurrentProfile(profile);
        performMovement(x, y, customProfile);
        updateFatigue();
        
        return true;
    }

    bool HumanPatterns::clickSequenceHumanLike(const std::vector<MouseButton>& buttons,
                                              Profile profile) {
        if (!m_device || !m_device->isConnected()) {
            return false;
        }

        CustomProfile customProfile = getCurrentProfile(profile);
        
        for (size_t i = 0; i < buttons.size(); ++i) {
            if (customProfile.jitter_probability > 0 && 
                m_distribution(m_rng) < customProfile.jitter_probability) {
                applyJitter(customProfile);
            }
            
            performClick(buttons[i], customProfile);
            updateFatigue();
            
            if (i < buttons.size() - 1) {
                auto sequenceDelay = getRandomTiming(customProfile.release_min, 
                                                   customProfile.release_max, customProfile);
                std::this_thread::sleep_for(sequenceDelay);
            }
        }
        
        return true;
    }

    bool HumanPatterns::dragHumanLike(int32_t start_x, int32_t start_y, 
                                     int32_t end_x, int32_t end_y, 
                                     MouseButton button, Profile profile) {
        if (!m_device || !m_device->isConnected()) {
            return false;
        }

        CustomProfile customProfile = getCurrentProfile(profile);
        
        performMovement(start_x, start_y, customProfile);
        
        auto pressDelay = getRandomTiming(customProfile.press_min, customProfile.press_max, customProfile);
        std::this_thread::sleep_for(pressDelay);
        
        m_device->mouseDown(button);
        
        auto path = generateMovementPath(start_x, start_y, end_x, end_y, customProfile);
        
        for (const auto& point : path) {
            performMovement(point.first, point.second, customProfile);
            auto moveDelay = getRandomTiming(std::chrono::milliseconds(8), 
                                           std::chrono::milliseconds(25), customProfile);
            std::this_thread::sleep_for(moveDelay);
        }
        
        auto releaseDelay = getRandomTiming(customProfile.release_min, customProfile.release_max, customProfile);
        std::this_thread::sleep_for(releaseDelay);
        
        m_device->mouseUp(button);
        updateFatigue();
        
        return true;
    }

    bool HumanPatterns::scrollHumanLike(int32_t delta, int scroll_steps, Profile profile) {
        if (!m_device || !m_device->isConnected()) {
            return false;
        }

        CustomProfile customProfile = getCurrentProfile(profile);
        
        int32_t step_size = delta / scroll_steps;
        int32_t remainder = delta % scroll_steps;
        
        for (int i = 0; i < scroll_steps; ++i) {
            int32_t current_delta = step_size;
            if (i < remainder) {
                current_delta += (delta > 0) ? 1 : -1;
            }
            
            if (customProfile.jitter_probability > 0 && 
                m_distribution(m_rng) < customProfile.jitter_probability * 0.5) {
                applyJitter(customProfile);
            }
            
            m_device->mouseWheel(current_delta);
            
            if (i < scroll_steps - 1) {
                auto scrollDelay = getRandomTiming(std::chrono::milliseconds(50), 
                                                 std::chrono::milliseconds(150), customProfile);
                std::this_thread::sleep_for(scrollDelay);
            }
        }
        
        updateFatigue();
        return true;
    }

    void HumanPatterns::setCustomProfile(const std::string& name, const CustomProfile& profile) {
        m_customProfiles[name] = profile;
    }

    bool HumanPatterns::setActiveProfile(const std::string& name) {
        if (m_customProfiles.find(name) != m_customProfiles.end()) {
            m_activeCustomProfile = name;
            return true;
        }
        return false;
    }

    void HumanPatterns::resetToDefaultProfile(Profile profile) {
        m_activeCustomProfile.clear();
    }

    void HumanPatterns::setSessionFatigue(double fatigue_level) {
        m_currentFatigue = std::clamp(fatigue_level, 0.0, 1.0);
    }

    std::string HumanPatterns::profileToString(Profile profile) {
        switch (profile) {
        case Profile::PRECISION: return "precision";
        case Profile::CASUAL: return "casual";
        case Profile::GAMING: return "gaming";
        case Profile::SLEEPY: return "sleepy";
        case Profile::CAFFEINATED: return "caffeinated";
        }
        return "casual";
    }

    HumanPatterns::Profile HumanPatterns::stringToProfile(const std::string& profileName) {
        std::string lower = profileName;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        
        if (lower == "precision") return Profile::PRECISION;
        if (lower == "gaming") return Profile::GAMING;
        if (lower == "sleepy") return Profile::SLEEPY;
        if (lower == "caffeinated") return Profile::CAFFEINATED;
        
        return Profile::CASUAL;
    }

    HumanPatterns::CustomProfile HumanPatterns::getCurrentProfile(Profile profile) const {
        if (!m_activeCustomProfile.empty()) {
            auto it = m_customProfiles.find(m_activeCustomProfile);
            if (it != m_customProfiles.end()) {
                return it->second;
            }
        }
        
        auto it = s_defaultProfiles.find(profile);
        if (it != s_defaultProfiles.end()) {
            return it->second;
        }
        return s_defaultProfiles.at(Profile::CASUAL);
    }

    std::chrono::milliseconds HumanPatterns::getRandomTiming(std::chrono::milliseconds min, 
                                                            std::chrono::milliseconds max,
                                                            const CustomProfile& profile) {
        if (min >= max) {
            return min;
        }
        
        auto base_range = max.count() - min.count();
        auto randomOffset = static_cast<int64_t>(m_distribution(m_rng) * base_range);
        auto result = std::chrono::milliseconds(min.count() + randomOffset);
        
        return applyVariation(result, profile);
    }

    void HumanPatterns::applyJitter(const CustomProfile& profile) {
        auto jitterOffset = generateJitterOffset(profile);
        m_device->mouseMove(jitterOffset.first, jitterOffset.second);
        
        auto jitterDelay = getRandomTiming(profile.jitter_delay_min, 
                                          profile.jitter_delay_max, profile);
        std::this_thread::sleep_for(jitterDelay);
    }

    void HumanPatterns::performClick(MouseButton button, const CustomProfile& profile) {
        auto pressTime = getRandomTiming(profile.press_min, profile.press_max, profile);
        
        m_device->mouseDown(button);
        std::this_thread::sleep_for(pressTime);
        m_device->mouseUp(button);
        
        m_actionCount++;
    }

    void HumanPatterns::performMovement(int32_t x, int32_t y, const CustomProfile& profile) {
        if (profile.movement_curve == MovementCurve::LINEAR || profile.movement_segments <= 1) {
            m_device->mouseMove(x, y);
        } else {
            auto path = generateMovementPath(0, 0, x, y, profile);
            for (const auto& point : path) {
                m_device->mouseMove(point.first, point.second);
                auto moveDelay = getRandomTiming(std::chrono::milliseconds(3), 
                                               std::chrono::milliseconds(8), profile);
                std::this_thread::sleep_for(moveDelay);
            }
        }
    }

    double HumanPatterns::calculateCurvePoint(double t, MovementCurve curve) const {
        switch (curve) {
        case MovementCurve::LINEAR:
            return t;
        case MovementCurve::EASE_IN:
            return t * t;
        case MovementCurve::EASE_OUT:
            return 1.0 - (1.0 - t) * (1.0 - t);
        case MovementCurve::EASE_IN_OUT:
            return t < 0.5 ? 2.0 * t * t : 1.0 - 2.0 * (1.0 - t) * (1.0 - t);
        case MovementCurve::BEZIER_NATURAL:
            return 3.0 * t * t - 2.0 * t * t * t;
        case MovementCurve::OVERSHOOT:
            return 1.0 + 0.3 * std::sin(t * M_PI);
        }
        return t;
    }

    std::chrono::milliseconds HumanPatterns::applyVariation(std::chrono::milliseconds base, 
                                                           const CustomProfile& profile) {
        double variation = 0.0;
        
        switch (profile.click_variation) {
        case ClickVariation::CONSISTENT:
            variation = m_normal_distribution(m_rng) * 0.02;
            break;
        case ClickVariation::SLIGHT_VARIATION:
            variation = m_normal_distribution(m_rng) * 0.05;
            break;
        case ClickVariation::MODERATE_VARIATION:
            variation = m_normal_distribution(m_rng) * 0.1;
            break;
        case ClickVariation::HIGH_VARIATION:
            variation = m_normal_distribution(m_rng) * 0.2;
            break;
        case ClickVariation::CHAOTIC:
            variation = m_normal_distribution(m_rng) * 0.4;
            break;
        }
        
        variation += profile.timing_drift * (m_distribution(m_rng) - 0.5);
        
        if (profile.enable_fatigue) {
            variation += m_currentFatigue * profile.fatigue_factor;
        }
        
        if (profile.use_acceleration && m_actionCount >= profile.acceleration_buildup) {
            double acceleration = std::min(1.0, static_cast<double>(m_actionCount) / 
                                         (profile.acceleration_buildup * 3));
            variation -= acceleration * profile.acceleration_factor * 0.1;
        }
        
        auto adjusted = static_cast<int64_t>(base.count() * (1.0 + variation));
        return std::chrono::milliseconds(std::max(static_cast<int64_t>(1), adjusted));
    }

    void HumanPatterns::updateFatigue() {
        m_actionCount++;
        
        for (const auto& [profile_enum, profile] : s_defaultProfiles) {
            if (profile.enable_fatigue && m_actionCount >= profile.fatigue_threshold) {
                m_currentFatigue = std::min(1.0, m_currentFatigue + 0.01);
                break;
            }
        }
    }


    std::pair<int32_t, int32_t> HumanPatterns::generateJitterOffset(const CustomProfile& profile) {
        if (profile.jitter_radius <= 0) {
            return {0, 0};
        }
        
        std::uniform_int_distribution<int> jitterDist(-profile.jitter_radius, profile.jitter_radius);
        return {jitterDist(m_rng), jitterDist(m_rng)};
    }

    std::vector<std::pair<int32_t, int32_t>> HumanPatterns::generateMovementPath(
        int32_t start_x, int32_t start_y, int32_t end_x, int32_t end_y, 
        const CustomProfile& profile) {
        
        std::vector<std::pair<int32_t, int32_t>> path;
        
        if (profile.movement_segments <= 1) {
            path.emplace_back(end_x - start_x, end_y - start_y);
            return path;
        }
        
        int32_t total_dx = end_x - start_x;
        int32_t total_dy = end_y - start_y;
        
        for (uint32_t i = 1; i <= profile.movement_segments; ++i) {
            double t = static_cast<double>(i) / profile.movement_segments;
            double curve_t = calculateCurvePoint(t, profile.movement_curve);
            
            int32_t segment_x = static_cast<int32_t>(total_dx * curve_t);
            int32_t segment_y = static_cast<int32_t>(total_dy * curve_t);
            
            double smoothness_noise = (m_distribution(m_rng) - 0.5) * 
                                     (1.0 - profile.smoothness_factor) * 10.0;
            segment_x += static_cast<int32_t>(smoothness_noise);
            segment_y += static_cast<int32_t>(smoothness_noise);
            
            if (i == 1) {
                path.emplace_back(segment_x, segment_y);
            } else {
                auto& prev = path.back();
                path.emplace_back(segment_x - prev.first, segment_y - prev.second);
            }
        }
        
        return path;
    }

} // namespace makcu