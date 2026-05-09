#include "sim/ability_definition.hpp"
#include "sim/ability_registry.hpp"
#include "sim/ability_system.hpp"
#include "sim/resolution_context.hpp"
#include "sim/sim_commands.hpp"
#include "sim/spatial_grid.hpp"
#include "sim/world.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace sim;
using Catch::Matchers::WithinAbs;

namespace {

constexpr SimFloat Tolerance = 0.01f;

AbilityId register_traveling_fireball(AbilityRegistry& registry, SimFloat travel_speed) {
    AbilityDefinition def{
        .name = "TravelingFireball",
        .tags = tags::Fire | tags::Spell | tags::Projectile,
        .cast_time_ticks = 0,
        .travel_speed = travel_speed,
        .effects = {},
    };
    def.effects.emplace_back(DamageEffect{
        .damage_stat = StatId::FireDamage,
        .base_amount = 50.0f,
    });
    return registry.register_ability(std::move(def));
}

}  // namespace

TEST_CASE("ability with travel speed enters traveling phase", "[travel]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_traveling_fireball(registry, 5.0f);

    auto caster = world.spawn(EntityKind::Player);
    world.transform(caster) = Transform{.x = 0.0f, .y = 0.0f};

    auto target = world.spawn(EntityKind::Enemy);
    world.transform(target) = Transform{.x = 50.0f, .y = 0.0f};
    world.health(target) = Health{.current = 200.0f, .max = 200.0f};

    ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 50.0f, 0.0f, ctx);

    REQUIRE(system.casting().empty());
    REQUIRE(system.traveling().size() == 1);
    REQUIRE(commands.deal_damage.empty());
}

TEST_CASE("projectile advances each tick", "[travel]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_traveling_fireball(registry, 5.0f);

    auto caster = world.spawn(EntityKind::Player);
    world.transform(caster) = Transform{.x = 0.0f, .y = 0.0f};

    auto target = world.spawn(EntityKind::Enemy);
    world.transform(target) = Transform{.x = 50.0f, .y = 0.0f};
    world.health(target) = Health{.current = 200.0f, .max = 200.0f};

    ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 50.0f, 0.0f, ctx);

    ctx.current_tick = 1;
    system.update(ctx);
    REQUIRE_THAT(system.traveling()[0].current_x, WithinAbs(5.0f, Tolerance));

    ctx.current_tick = 2;
    system.update(ctx);
    REQUIRE_THAT(system.traveling()[0].current_x, WithinAbs(10.0f, Tolerance));
}

TEST_CASE("projectile resolves on arrival", "[travel]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_traveling_fireball(registry, 50.0f);

    auto caster = world.spawn(EntityKind::Player);
    world.transform(caster) = Transform{.x = 0.0f, .y = 0.0f};

    auto target = world.spawn(EntityKind::Enemy);
    world.transform(target) = Transform{.x = 100.0f, .y = 0.0f};
    world.health(target) = Health{.current = 200.0f, .max = 200.0f};

    ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 100.0f, 0.0f, ctx);

    ctx.current_tick = 1;
    system.update(ctx);
    REQUIRE(system.traveling().size() == 1);

    ctx.current_tick = 2;
    system.update(ctx);
    REQUIRE(system.traveling().empty());
    REQUIRE(commands.deal_damage.size() == 1);
}

TEST_CASE("projectile with cast time goes through both phases", "[travel]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    AbilityDefinition def{
        .name = "ChannelledFireball",
        .tags = tags::Fire | tags::Spell,
        .cast_time_ticks = 3,
        .travel_speed = 25.0f,
        .effects = {},
    };
    def.effects.emplace_back(DamageEffect{
        .damage_stat = StatId::FireDamage,
        .base_amount = 50.0f,
    });
    const AbilityId fireball = registry.register_ability(std::move(def));

    auto caster = world.spawn(EntityKind::Player);
    world.transform(caster) = Transform{.x = 0.0f, .y = 0.0f};

    auto target = world.spawn(EntityKind::Enemy);
    world.transform(target) = Transform{.x = 50.0f, .y = 0.0f};
    world.health(target) = Health{.current = 200.0f, .max = 200.0f};

    ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 50.0f, 0.0f, ctx);

    REQUIRE(system.casting().size() == 1);
    REQUIRE(system.traveling().empty());

    ctx.current_tick = 3;
    system.update(ctx);
    REQUIRE(system.casting().empty());
    REQUIRE(system.traveling().size() == 1);

    ctx.current_tick = 4;
    system.update(ctx);
    REQUIRE(system.traveling().empty());
    REQUIRE(commands.deal_damage.size() == 1);
}

TEST_CASE("projectile travels diagonally", "[travel]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_traveling_fireball(registry, 5.0f);

    auto caster = world.spawn(EntityKind::Player);
    world.transform(caster) = Transform{.x = 0.0f, .y = 0.0f};

    auto target = world.spawn(EntityKind::Enemy);
    world.transform(target) = Transform{.x = 30.0f, .y = 40.0f};
    world.health(target) = Health{.current = 200.0f, .max = 200.0f};

    ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 30.0f, 40.0f, ctx);

    ctx.current_tick = 1;
    system.update(ctx);

    REQUIRE_THAT(system.traveling()[0].current_x, WithinAbs(3.0f, Tolerance));
    REQUIRE_THAT(system.traveling()[0].current_y, WithinAbs(4.0f, Tolerance));
}