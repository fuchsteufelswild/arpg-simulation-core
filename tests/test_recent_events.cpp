#include "sim/recent_events.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace sim;

TEST_CASE("default RecentEvents has no events", "[recent_events]") {
    RecentEvents events;
    REQUIRE_FALSE(events.any_within(EventType::CritHit, 100, 60));
    REQUIRE_FALSE(events.any_within(EventType::TookDamage, 100, 60));
}

TEST_CASE("event within window is detected", "[recent_events]") {
    RecentEvents events;
    events.push(EventType::CritHit, 100);

    REQUIRE(events.any_within(EventType::CritHit, 110, 60));
    REQUIRE(events.any_within(EventType::CritHit, 100, 0));
}

TEST_CASE("event outside window is not detected", "[recent_events]") {
    RecentEvents events;
    events.push(EventType::CritHit, 100);

    REQUIRE_FALSE(events.any_within(EventType::CritHit, 200, 50));
}

TEST_CASE("event of different type is not detected", "[recent_events]") {
    RecentEvents events;
    events.push(EventType::TookDamage, 100);

    REQUIRE_FALSE(events.any_within(EventType::CritHit, 110, 60));
}

TEST_CASE("ring buffer overwrites oldest entries", "[recent_events]") {
    RecentEvents events;
    events.push(EventType::CritHit, 10);

    for (int i = 0; i < 20; ++i) {
        events.push(EventType::TookDamage, 100 + i);
    }

    REQUIRE_FALSE(events.any_within(EventType::CritHit, 200, 1000));
    REQUIRE(events.any_within(EventType::TookDamage, 200, 100));
}

TEST_CASE("clear removes all events", "[recent_events]") {
    RecentEvents events;
    events.push(EventType::CritHit, 100);
    events.clear();

    REQUIRE_FALSE(events.any_within(EventType::CritHit, 110, 60));
}

TEST_CASE("query at tick 0 with window does not underflow", "[recent_events]") {
    RecentEvents events;
    events.push(EventType::CritHit, 0);

    REQUIRE(events.any_within(EventType::CritHit, 0, 100));
}