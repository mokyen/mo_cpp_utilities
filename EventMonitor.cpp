#include <chrono>
#include <optional>

// Pure decision-making functions
namespace EventMonitorHelpers {
    // Decide if we have a timeout condition based on the optional last event time
    bool isTimeoutExceeded(
        const std::optional<std::chrono::steady_clock::time_point>& lastEventTime,
        const std::chrono::steady_clock::time_point& currentTime,
        const std::chrono::milliseconds& threshold) {
        // If we've never received an event, we consider this a timeout condition
        if (!lastEventTime) {
            return true;
        }

        auto timeSinceLastEvent = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - *lastEventTime);
        return timeSinceLastEvent > threshold;
    }
}

class EventMonitor {
public:
    // Constructor only initializes the threshold now
    explicit EventMonitor(std::chrono::milliseconds timeoutThreshold) 
        : m_timeoutThreshold(timeoutThreshold) {}

    // Record a new event occurrence
    void recordEvent() {
        m_lastEventTime = std::chrono::steady_clock::now();
    }

    // Check if we've exceeded the timeout threshold or haven't received an event yet
    bool hasTimeoutOccurred() const {
        // Wiring: gather current time, use decision function, return result
        auto currentTime = std::chrono::steady_clock::now();
        return EventMonitorHelpers::isTimeoutExceeded(
            m_lastEventTime, 
            currentTime, 
            m_timeoutThreshold);
    }

    // Check if we've ever received an event
    bool hasEventBeenReceived() const {
        return m_lastEventTime.has_value();
    }

    // Allow updating the threshold if needed
    void updateTimeoutThreshold(std::chrono::milliseconds newThreshold) {
        m_timeoutThreshold = newThreshold;
    }

    // Get the time since last event (if any)
    std::optional<std::chrono::milliseconds> getTimeSinceLastEvent() const {
        if (!m_lastEventTime) {
            return std::nullopt;
        }
        
        auto currentTime = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - *m_lastEventTime);
    }

private:
    // The threshold we're checking against
    std::chrono::milliseconds m_timeoutThreshold;
    
    // The last time an event occurred (nullopt if no event received yet)
    std::optional<std::chrono::steady_clock::time_point> m_lastEventTime;
};
