#include "sim/world.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace sim;

TEST_CASE("default-constructed handle is null", "[world]") {
    EntityHandle h;
    REQUIRE(h.is_null());
}

TEST_CASE("spawn produces valid handles", "[world]") {
    World world;
    auto h = world.spawn(EntityKind::Enemy);

    REQUIRE_FALSE(h.is_null());
    REQUIRE(world.is_alive(h));
    REQUIRE(world.kind(h) == EntityKind::Enemy);
    REQUIRE(world.alive_count() == 1);
}

TEST_CASE("kill invalidates the handle", "[world]") {
    World world;
    auto h = world.spawn(EntityKind::Enemy);
    REQUIRE(world.is_alive(h));

    world.kill(h);

    REQUIRE_FALSE(world.is_alive(h));
    REQUIRE(world.alive_count() == 0);
}

TEST_CASE("freelist reuses indices", "[world]") {
    World world;
    auto h1 = world.spawn(EntityKind::Enemy);
    const uint32_t original_index = h1.index;

    world.kill(h1);
    auto h2 = world.spawn(EntityKind::Enemy);

    REQUIRE(h2.index == original_index);
    REQUIRE(h2.generation != h1.generation);
}

TEST_CASE("stale handle does not match reused slot", "[world]") {
    World world;
    auto h1 = world.spawn(EntityKind::Enemy);
    world.kill(h1);
    auto h2 = world.spawn(EntityKind::Player);

    REQUIRE_FALSE(world.is_alive(h1));
    REQUIRE(world.is_alive(h2));
    REQUIRE(world.kind(h2) == EntityKind::Player);
}

TEST_CASE("kill on already-dead handle is safe", "[world]") {
    World world;
    auto h = world.spawn(EntityKind::Enemy);
    world.kill(h);

    REQUIRE_NOTHROW(world.kill(h));
    REQUIRE(world.alive_count() == 0);
}

TEST_CASE("spawned entity has default-initialized components", "[world]") {
    World world;
    auto h = world.spawn(EntityKind::Player);

    REQUIRE(world.transform(h).x == 0.0f);
    REQUIRE(world.transform(h).y == 0.0f);
    REQUIRE(world.health(h).current == 0.0f);
    REQUIRE(world.health(h).max == 0.0f);
}

TEST_CASE("reused slot has reset components", "[world]") {
    World world;
    auto h1 = world.spawn(EntityKind::Enemy);
    world.transform(h1).x = 100.0f;
    world.health(h1).current = 50.0f;

    world.kill(h1);
    auto h2 = world.spawn(EntityKind::Enemy);

    REQUIRE(world.transform(h2).x == 0.0f);
    REQUIRE(world.health(h2).current == 0.0f);
}

TEST_CASE("multiple spawns produce distinct handles", "[world]") {
    World world;
    auto h1 = world.spawn(EntityKind::Enemy);
    auto h2 = world.spawn(EntityKind::Enemy);
    auto h3 = world.spawn(EntityKind::Enemy);

    REQUIRE(h1 != h2);
    REQUIRE(h1 != h3);
    REQUIRE(world.alive_count() == 3);
}