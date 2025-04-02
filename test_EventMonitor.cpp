#include <gtest/gtest.h>
#include <chrono>
#include <optional>

// Helper function tests
class EventMonitorHelperTests : public ::testing::Test {};

TEST_F(EventMonitorHelperTests, WhenNoEventReceived_ReturnsTrue) {
    std::optional<std::chrono::steady_clock::time_point> noTime;
    auto currentTime = std::chrono::steady_clock::now();
    auto threshold = std::chrono::milliseconds(50);

    EXPECT_TRUE(EventMonitorHelpers::isTimeoutExceeded(noTime, currentTime, threshold));
}

TEST_F(EventMonitorHelperTests, WhenTimeExceedsThreshold_ReturnsTrue) {
    auto baseTime = std::chrono::steady_clock::now();
    auto laterTime = baseTime + std::chrono::milliseconds(100);
    auto threshold = std::chrono::milliseconds(50);

    EXPECT_TRUE(EventMonitorHelpers::isTimeoutExceeded(baseTime, laterTime, threshold));
}

TEST_F(EventMonitorHelperTests, WhenTimeWithinThreshold_ReturnsFalse) {
    auto baseTime = std::chrono::steady_clock::now();
    auto laterTime = baseTime + std::chrono::milliseconds(30);
    auto threshold = std::chrono::milliseconds(50);

    EXPECT_FALSE(EventMonitorHelpers::isTimeoutExceeded(baseTime, laterTime, threshold));
}

TEST_F(EventMonitorHelperTests, WhenTimeExactlyAtThreshold_ReturnsFalse) {
    auto baseTime = std::chrono::steady_clock::now();
    auto laterTime = baseTime + std::chrono::milliseconds(50);
    auto threshold = std::chrono::milliseconds(50);

    EXPECT_FALSE(EventMonitorHelpers::isTimeoutExceeded(baseTime, laterTime, threshold));
}

// EventMonitor class tests
class EventMonitorTests : public ::testing::Test {
protected:
    const std::chrono::milliseconds kDefaultTimeout{1000};
};

// Initial state tests
TEST_F(EventMonitorTests, WhenInitialized_HasNoEventReceived) {
    EventMonitor monitor(kDefaultTimeout);
    EXPECT_FALSE(monitor.hasEventBeenReceived());
}

TEST_F(EventMonitorTests, WhenInitialized_TimeoutHasOccurred) {
    EventMonitor monitor(kDefaultTimeout);
    EXPECT_TRUE(monitor.hasTimeoutOccurred());
}

TEST_F(EventMonitorTests, WhenInitialized_GetTimeSinceLastEventReturnsNullopt) {
    EventMonitor monitor(kDefaultTimeout);
    EXPECT_FALSE(monitor.getTimeSinceLastEvent().has_value());
}

// State transition tests
TEST_F(EventMonitorTests, AfterRecordingEvent_HasEventBeenReceivedIsTrue) {
    EventMonitor monitor(kDefaultTimeout);
    monitor.recordEvent();
    EXPECT_TRUE(monitor.hasEventBeenReceived());
}

TEST_F(EventMonitorTests, AfterRecordingEvent_TimeoutHasNotOccurred) {
    EventMonitor monitor(kDefaultTimeout);
    monitor.recordEvent();
    EXPECT_FALSE(monitor.hasTimeoutOccurred());
}

TEST_F(EventMonitorTests, AfterRecordingEvent_GetTimeSinceLastEventReturnsValue) {
    EventMonitor monitor(kDefaultTimeout);
    monitor.recordEvent();
    EXPECT_TRUE(monitor.getTimeSinceLastEvent().has_value());
}

// Threshold update tests
TEST_F(EventMonitorTests, WhenThresholdUpdated_ValueIsRespected) {
    EventMonitor monitor(std::chrono::milliseconds(1000));
    monitor.recordEvent();
    
    // Update to a very large threshold
    monitor.updateTimeoutThreshold(std::chrono::milliseconds(100000));
    EXPECT_FALSE(monitor.hasTimeoutOccurred());
    
    // Update to a very small threshold (should cause timeout)
    monitor.updateTimeoutThreshold(std::chrono::milliseconds(1));
    EXPECT_TRUE(monitor.hasTimeoutOccurred());
}
