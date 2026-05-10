#include "sim_api/sim_api.h"

#include <catch2/catch_test_macros.hpp>

#include <cstring>

TEST_CASE("sim_version returns non-empty string", "[sim_api]") {
    const char* version = sim_version();
    REQUIRE(version != nullptr);
    REQUIRE(std::strlen(version) > 0);
}

TEST_CASE("sim_create returns valid handle", "[sim_api]") {
    SimHandle sim = sim_create(42);
    REQUIRE(sim != nullptr);
    sim_destroy(sim);
}

TEST_CASE("sim_destroy on nullptr is safe", "[sim_api]") {
    sim_destroy(nullptr);
}

TEST_CASE("sim_advance increments tick via snapshot", "[sim_api]") {
    SimHandle sim = sim_create(42);

    Snapshot snap{};
    sim_get_snapshot(sim, &snap);
    REQUIRE(snap.tick == 0);

    sim_advance(sim, 5);
    sim_get_snapshot(sim, &snap);
    REQUIRE(snap.tick == 5);

    sim_destroy(sim);
}

TEST_CASE("snapshot reflects entity count", "[sim_api]") {
    SimHandle sim = sim_create(42);

    Snapshot snap{};
    sim_get_snapshot(sim, &snap);
    REQUIRE(snap.entity_count == 0);

    sim_destroy(sim);
}

TEST_CASE("submit_commands with null buffer is safe", "[sim_api]") {
    SimHandle sim = sim_create(42);
    sim_submit_commands(sim, nullptr);
    sim_destroy(sim);
}

TEST_CASE("submit_commands with empty buffer is safe", "[sim_api]") {
    SimHandle sim = sim_create(42);

    InputCmdBuffer buffer{};
    buffer.count = 0;
    buffer.commands = nullptr;
    sim_submit_commands(sim, &buffer);

    sim_destroy(sim);
}

TEST_CASE("get_snapshot with null out is safe", "[sim_api]") {
    SimHandle sim = sim_create(42);
    sim_get_snapshot(sim, nullptr);
    sim_destroy(sim);
}

TEST_CASE("get_snapshot on null handle returns empty", "[sim_api]") {
    Snapshot snap{};
    snap.tick = 999;
    snap.entity_count = 999;

    sim_get_snapshot(nullptr, &snap);

    REQUIRE(snap.tick == 0);
    REQUIRE(snap.entity_count == 0);
    REQUIRE(snap.entities == nullptr);
}

TEST_CASE("two sims are independent", "[sim_api]") {
    SimHandle sim_a = sim_create(1);
    SimHandle sim_b = sim_create(2);

    sim_advance(sim_a, 10);
    sim_advance(sim_b, 5);

    Snapshot snap_a{};
    Snapshot snap_b{};
    sim_get_snapshot(sim_a, &snap_a);
    sim_get_snapshot(sim_b, &snap_b);

    REQUIRE(snap_a.tick == 10);
    REQUIRE(snap_b.tick == 5);

    sim_destroy(sim_a);
    sim_destroy(sim_b);
}

TEST_CASE("EntitySnapshot has expected size", "[sim_api][layout]") {
    REQUIRE(sizeof(EntitySnapshot) == 48);
}

TEST_CASE("Snapshot has expected size", "[sim_api][layout]") {
    REQUIRE(sizeof(Snapshot) == 24);
}

TEST_CASE("CastAbilityCmd has expected size", "[sim_api][layout]") {
    REQUIRE(sizeof(CastAbilityCmd) == 28);
}

TEST_CASE("MoveIntentCmd has expected size", "[sim_api][layout]") {
    REQUIRE(sizeof(MoveIntentCmd) == 16);
}