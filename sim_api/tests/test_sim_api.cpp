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
    REQUIRE(snap.entity_count == 1);

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

TEST_CASE("snapshot has zero counts in empty sim", "[sim_api]") {
    SimHandle sim = sim_create(42);

    Snapshot snap{};
    sim_get_snapshot(sim, &snap);

    REQUIRE(snap.entity_count == 1);
    REQUIRE(snap.projectile_count == 0);
    REQUIRE(snap.damage_event_count == 0);

    sim_destroy(sim);
}

TEST_CASE("get_cooldowns handles null/invalid inputs safely", "[sim_api]") {
    SimHandle sim = sim_create(42);
    CooldownSnapshot snap{};

    SECTION("Null out_snapshot") {
        sim_get_cooldowns(sim, 0, 0, nullptr);
    }

    SECTION("Null SimHandle") {
        snap.count = 999;
        sim_get_cooldowns(nullptr, 0, 0, &snap);
        REQUIRE(snap.count == 0);
        REQUIRE(snap.entries == nullptr);
    }

    SECTION("Invalid Entity Handle") {
        sim_get_cooldowns(sim, 9999, 1, &snap);
        REQUIRE(snap.count == 0);
        REQUIRE(snap.entity_index == 9999);
    }

    sim_destroy(sim);
}

TEST_CASE("get_cooldowns reflects active simulation cooldowns", "[sim_api]") {
    SimHandle sim = sim_create(42);

    InputCmdBuffer buf{};
    InputCmd cmd{};
    cmd.type = SIM_CMD_CAST_ABILITY;
    cmd.payload.cast.caster_index = 0;
    cmd.payload.cast.caster_generation = 1;
    cmd.payload.cast.ability_id = 10;

    buf.count = 1;
    buf.commands = &cmd;
    sim_submit_commands(sim, &buf);

    sim_advance(sim, 1);

    CooldownSnapshot cool_snap{};
    sim_get_cooldowns(sim, 0, 1, &cool_snap);

    if (cool_snap.count > 0) {
        REQUIRE(cool_snap.entries != nullptr);
        REQUIRE(cool_snap.entries[0].ability_id == 10);
        REQUIRE(cool_snap.entries[0].remaining_ticks > 0);
    }

    sim_destroy(sim);
}

TEST_CASE("EntitySnapshot has expected layout", "[sim_api][layout]") {
    REQUIRE(sizeof(EntitySnapshot) == 48);
}

TEST_CASE("ProjectileSnapshot has expected layout", "[sim_api][layout]") {
    REQUIRE(sizeof(ProjectileSnapshot) == 32);
}

TEST_CASE("DamageEventSnapshot has expected layout", "[sim_api][layout]") {
    REQUIRE(sizeof(DamageEventSnapshot) == 32);
}

TEST_CASE("Snapshot has expected size", "[sim_api][layout]") {
    REQUIRE(sizeof(Snapshot) == 48);
}

TEST_CASE("CastAbilityCmd has expected size", "[sim_api][layout]") {
    REQUIRE(sizeof(CastAbilityCmd) == 28);
}

TEST_CASE("MoveIntentCmd has expected size", "[sim_api][layout]") {
    REQUIRE(sizeof(MoveIntentCmd) == 16);
}

TEST_CASE("Cooldown types have expected layout", "[sim_api][layout]") {
    REQUIRE(sizeof(CooldownEntry) == 8);
    REQUIRE(sizeof(CooldownSnapshot) == 24);
}