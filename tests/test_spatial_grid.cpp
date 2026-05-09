#include "sim/entity_handle.hpp"
#include "sim/spatial_grid.hpp"
#include "sim/world.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace sim;

TEST_CASE("empty grid returns no results", "[spatial]") {
    SpatialGrid grid;
    World world;
    grid.rebuild(world);

    REQUIRE(grid.entry_count() == 0);
    REQUIRE(grid.query_within(0.0f, 0.0f, 100.0f).empty());
}

TEST_CASE("single entity within radius is found", "[spatial]") {
    SpatialGrid grid;
    World world;
    auto entity = world.spawn(EntityKind::Enemy);
    world.transform(entity) = Transform{.x = 5.0f, .y = 5.0f};

    grid.rebuild(world);

    const auto results = grid.query_within(0.0f, 0.0f, 10.0f);
    REQUIRE(results.size() == 1);
    REQUIRE(results[0] == entity);
}

TEST_CASE("entity outside radius is excluded", "[spatial]") {
    SpatialGrid grid;
    World world;
    auto entity = world.spawn(EntityKind::Enemy);
    world.transform(entity) = Transform{.x = 50.0f, .y = 50.0f};

    grid.rebuild(world);

    const auto results = grid.query_within(0.0f, 0.0f, 10.0f);
    REQUIRE(results.empty());
}

TEST_CASE("results are sorted nearest-first", "[spatial]") {
    SpatialGrid grid;
    World world;
    auto far_entity = world.spawn(EntityKind::Enemy);
    auto near_entity = world.spawn(EntityKind::Enemy);
    auto medium_entity = world.spawn(EntityKind::Enemy);

    world.transform(far_entity) = Transform{.x = 8.0f, .y = 0.0f};
    world.transform(near_entity) = Transform{.x = 2.0f, .y = 0.0f};
    world.transform(medium_entity) = Transform{.x = 5.0f, .y = 0.0f};

    grid.rebuild(world);

    const auto results = grid.query_within(0.0f, 0.0f, 20.0f);
    REQUIRE(results.size() == 3);
    REQUIRE(results[0] == near_entity);
    REQUIRE(results[1] == medium_entity);
    REQUIRE(results[2] == far_entity);
}

TEST_CASE("entities at equal distance are tie-broken deterministically", "[spatial]") {
    SpatialGrid grid;
    World world;
    auto a = world.spawn(EntityKind::Enemy);
    auto b = world.spawn(EntityKind::Enemy);

    world.transform(a) = Transform{.x = 5.0f, .y = 0.0f};
    world.transform(b) = Transform{.x = -5.0f, .y = 0.0f};

    grid.rebuild(world);

    const auto results = grid.query_within(0.0f, 0.0f, 10.0f);
    REQUIRE(results.size() == 2);
    REQUIRE(results[0].index < results[1].index);
}

TEST_CASE("query at zero radius returns empty", "[spatial]") {
    SpatialGrid grid;
    World world;
    auto entity = world.spawn(EntityKind::Enemy);

    grid.rebuild(world);

    REQUIRE(grid.query_within(0.0f, 0.0f, 0.0f).empty());
    REQUIRE(grid.query_within(0.0f, 0.0f, -1.0f).empty());
}

TEST_CASE("query crossing cell boundaries finds entities in multiple cells", "[spatial]") {
    SpatialGrid grid(4.0f);
    World world;

    auto a = world.spawn(EntityKind::Enemy);
    auto b = world.spawn(EntityKind::Enemy);
    auto c = world.spawn(EntityKind::Enemy);

    world.transform(a) = Transform{.x = 1.0f, .y = 1.0f};
    world.transform(b) = Transform{.x = 5.0f, .y = 5.0f};
    world.transform(c) = Transform{.x = 9.0f, .y = 9.0f};

    grid.rebuild(world);

    const auto results = grid.query_within(5.0f, 5.0f, 10.0f);
    REQUIRE(results.size() == 3);
}

TEST_CASE("entities at negative coordinates are found", "[spatial]") {
    SpatialGrid grid;
    World world;
    auto entity = world.spawn(EntityKind::Enemy);
    world.transform(entity) = Transform{.x = -5.0f, .y = -5.0f};

    grid.rebuild(world);

    const auto results = grid.query_within(0.0f, 0.0f, 10.0f);
    REQUIRE(results.size() == 1);
    REQUIRE(results[0] == entity);
}

TEST_CASE("rebuild reflects new positions", "[spatial]") {
    SpatialGrid grid;
    World world;
    auto entity = world.spawn(EntityKind::Enemy);

    world.transform(entity) = Transform{.x = 100.0f, .y = 100.0f};
    grid.rebuild(world);
    REQUIRE(grid.query_within(0.0f, 0.0f, 50.0f).empty());

    world.transform(entity) = Transform{.x = 5.0f, .y = 5.0f};
    grid.rebuild(world);
    REQUIRE(grid.query_within(0.0f, 0.0f, 50.0f).size() == 1);
}

TEST_CASE("dead entities are not in the grid after rebuild", "[spatial]") {
    SpatialGrid grid;
    World world;
    auto entity = world.spawn(EntityKind::Enemy);
    world.transform(entity) = Transform{.x = 5.0f, .y = 5.0f};

    grid.rebuild(world);
    REQUIRE(grid.entry_count() == 1);

    world.kill(entity);
    grid.rebuild(world);
    REQUIRE(grid.entry_count() == 0);
}