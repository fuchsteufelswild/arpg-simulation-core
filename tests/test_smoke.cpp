#include "sim/version.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("sim version is non-empty", "[smoke]") {
    REQUIRE_FALSE(sim::version().empty());
}