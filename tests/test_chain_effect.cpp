#include "sim/ability_definition.hpp"
#include "sim/ability_registry.hpp"
#include "sim/ability_system.hpp"
#include "sim/sim_commands.hpp"
#include "sim/spatial_grid.hpp"
#include "sim/world.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace sim;
using Catch::Matchers::WithinAbs;

namespace {

constexpr SimFloat Tolerance = 0.0001f;

AbilityId register_chain_fireball(AbilityRegistry& registry,
                                  uint8_t max_chains,
                                  SimFloat falloff,
                                  SimFloat radius) {
    AbilityDefinition def{
        .name = "ChainFireball",
        .tags = tags::Fire | tags::Spell,
        .cast_time_ticks = 0,
        .effects = {},
    };
    def.effects.emplace_back(DamageEffect{
        .damage_stat = StatId::FireDamage,
        .base_amount = 100.0f,
    });
    def.effects.emplace_back(ChainEffect{
        .max_chains = max_chains,
        .falloff = falloff,
        .radius = radius,
    });
    return registry.register_ability(std::move(def));
}

}  // namespace

TEST_CASE("chain hits one nearby enemy with falloff", "[chain]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_chain_fireball(registry, 1, 0.5f, 10.0f);

    auto caster = world.spawn(EntityKind::Player);
    world.transform(caster) = Transform{.x = 0.0f, .y = 0.0f};

    auto target1 = world.spawn(EntityKind::Enemy);
    world.transform(target1) = Transform{.x = 5.0f, .y = 0.0f};
    world.health(target1) = Health{.current = 200.0f, .max = 200.0f};

    auto target2 = world.spawn(EntityKind::Enemy);
    world.transform(target2) = Transform{.x = 8.0f, .y = 0.0f};
    world.health(target2) = Health{.current = 200.0f, .max = 200.0f};

    grid.rebuild(world);

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target1, 5.0f, 0.0f, ctx);

    REQUIRE(commands.deal_damage.size() == 2);
    REQUIRE(commands.deal_damage[0].target == target1);
    REQUIRE_THAT(commands.deal_damage[0].amount, WithinAbs(100.0f, Tolerance));
    REQUIRE(commands.deal_damage[1].target == target2);
    REQUIRE_THAT(commands.deal_damage[1].amount, WithinAbs(50.0f, Tolerance));
}

TEST_CASE("chain stops when no more targets in radius", "[chain]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_chain_fireball(registry, 5, 0.8f, 5.0f);

    auto caster = world.spawn(EntityKind::Player);
    world.transform(caster) = Transform{.x = 0.0f, .y = 0.0f};

    auto target1 = world.spawn(EntityKind::Enemy);
    world.transform(target1) = Transform{.x = 5.0f, .y = 0.0f};

    auto target2 = world.spawn(EntityKind::Enemy);
    world.transform(target2) = Transform{.x = 8.0f, .y = 0.0f};

    grid.rebuild(world);

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target1, 5.0f, 0.0f, ctx);

    REQUIRE(commands.deal_damage.size() == 2);
}

TEST_CASE("chain doesn't bounce back to already-hit target", "[chain]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_chain_fireball(registry, 5, 1.0f, 100.0f);

    auto caster = world.spawn(EntityKind::Player);
    world.transform(caster) = Transform{.x = 0.0f, .y = 0.0f};

    auto target1 = world.spawn(EntityKind::Enemy);
    world.transform(target1) = Transform{.x = 5.0f, .y = 0.0f};

    auto target2 = world.spawn(EntityKind::Enemy);
    world.transform(target2) = Transform{.x = 8.0f, .y = 0.0f};

    grid.rebuild(world);

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target1, 5.0f, 0.0f, ctx);

    REQUIRE(commands.deal_damage.size() == 2);
    REQUIRE(commands.deal_damage[0].target == target1);
    REQUIRE(commands.deal_damage[1].target == target2);
}

TEST_CASE("chain skips caster", "[chain]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_chain_fireball(registry, 3, 1.0f, 100.0f);

    auto caster = world.spawn(EntityKind::Player);
    world.transform(caster) = Transform{.x = 0.0f, .y = 0.0f};

    auto target1 = world.spawn(EntityKind::Enemy);
    world.transform(target1) = Transform{.x = 5.0f, .y = 0.0f};

    grid.rebuild(world);

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target1, 5.0f, 0.0f, ctx);

    REQUIRE(commands.deal_damage.size() == 1);
    REQUIRE(commands.deal_damage[0].target == target1);
}

TEST_CASE("falloff applies cumulatively over hops", "[chain]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_chain_fireball(registry, 3, 0.5f, 100.0f);

    auto caster = world.spawn(EntityKind::Player);
    world.transform(caster) = Transform{.x = 0.0f, .y = 0.0f};

    auto target1 = world.spawn(EntityKind::Enemy);
    auto target2 = world.spawn(EntityKind::Enemy);
    auto target3 = world.spawn(EntityKind::Enemy);
    auto target4 = world.spawn(EntityKind::Enemy);

    world.transform(target1) = Transform{.x = 5.0f, .y = 0.0f};
    world.transform(target2) = Transform{.x = 10.0f, .y = 0.0f};
    world.transform(target3) = Transform{.x = 15.0f, .y = 0.0f};
    world.transform(target4) = Transform{.x = 20.0f, .y = 0.0f};

    grid.rebuild(world);

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target1, 5.0f, 0.0f, ctx);

    REQUIRE(commands.deal_damage.size() == 4);
    REQUIRE_THAT(commands.deal_damage[0].amount, WithinAbs(100.0f, Tolerance));
    REQUIRE_THAT(commands.deal_damage[1].amount, WithinAbs(50.0f, Tolerance));
    REQUIRE_THAT(commands.deal_damage[2].amount, WithinAbs(25.0f, Tolerance));
    REQUIRE_THAT(commands.deal_damage[3].amount, WithinAbs(12.5f, Tolerance));
}