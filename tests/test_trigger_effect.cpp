#include "sim/ability_definition.hpp"
#include "sim/ability_registry.hpp"
#include "sim/ability_system.hpp"
#include "sim/command_application.hpp"
#include "sim/resolution_context.hpp"
#include "sim/sim_commands.hpp"
#include "sim/spatial_grid.hpp"
#include "sim/world.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace sim;
using Catch::Matchers::WithinAbs;

namespace {

constexpr SimFloat Tolerance = 0.0001f;

}

TEST_CASE("trigger queues a cast command", "[trigger]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    AbilityDefinition ice_def{
        .name = "IceNova",
        .tags = tags::Cold | tags::Spell,
        .effects = {},
    };
    ice_def.effects.emplace_back(DamageEffect{
        .damage_stat = StatId::ColdDamage,
        .base_amount = 50.0f,
    });
    const AbilityId ice_nova = registry.register_ability(std::move(ice_def));

    AbilityDefinition fire_def{
        .name = "Fireball",
        .tags = tags::Fire | tags::Spell,
        .effects = {},
    };
    fire_def.effects.emplace_back(DamageEffect{
        .damage_stat = StatId::FireDamage,
        .base_amount = 100.0f,
    });
    fire_def.effects.emplace_back(TriggerEffect{
        .ability_to_trigger = ice_nova,
        .chance = 1.0f,
    });
    const AbilityId fireball = registry.register_ability(std::move(fire_def));

    auto caster = world.spawn(EntityKind::Player);
    auto target = world.spawn(EntityKind::Enemy);
    world.health(target) = Health{.current = 200.0f, .max = 200.0f};

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 5.0f, 0.0f, ctx);

    REQUIRE(commands.deal_damage.size() == 1);
    REQUIRE_THAT(commands.deal_damage[0].amount, WithinAbs(100.0f, Tolerance));
    REQUIRE(commands.cast_ability.size() == 1);
    REQUIRE(commands.cast_ability[0].ability_id == ice_nova);
    REQUIRE(commands.cast_ability[0].caster == caster);
}

TEST_CASE("trigger resolves on next tick via apply_commands", "[trigger]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;
    std::vector<DamageEvent> damage_events;

    AbilityDefinition ice_def{
        .name = "IceNova",
        .tags = tags::Cold | tags::Spell,
        .effects = {},
    };
    ice_def.effects.emplace_back(DamageEffect{
        .damage_stat = StatId::ColdDamage,
        .base_amount = 50.0f,
    });
    const AbilityId ice_nova = registry.register_ability(std::move(ice_def));

    AbilityDefinition fire_def{
        .name = "Fireball",
        .tags = tags::Fire | tags::Spell,
        .effects = {},
    };
    fire_def.effects.emplace_back(DamageEffect{
        .damage_stat = StatId::FireDamage,
        .base_amount = 100.0f,
    });
    fire_def.effects.emplace_back(TriggerEffect{
        .ability_to_trigger = ice_nova,
        .chance = 1.0f,
    });
    const AbilityId fireball = registry.register_ability(std::move(fire_def));

    auto caster = world.spawn(EntityKind::Player);
    auto target = world.spawn(EntityKind::Enemy);
    world.health(target) = Health{.current = 500.0f, .max = 500.0f};

    ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 0.0f, 0.0f, ctx);
    apply_commands(system, ctx, &damage_events);

    REQUIRE(commands.empty());
    REQUIRE_THAT(world.health(target).current, WithinAbs(350.0f, Tolerance));
}

TEST_CASE("trigger with chance < 1 does not fire", "[trigger]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    AbilityDefinition ice_def{.name = "IceNova", .effects = {}};
    const AbilityId ice_nova = registry.register_ability(std::move(ice_def));

    AbilityDefinition fire_def{.name = "Fireball", .effects = {}};
    fire_def.effects.emplace_back(TriggerEffect{
        .ability_to_trigger = ice_nova,
        .chance = 0.5f,
    });
    const AbilityId fireball = registry.register_ability(std::move(fire_def));

    auto caster = world.spawn(EntityKind::Player);
    auto target = world.spawn(EntityKind::Enemy);

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 0.0f, 0.0f, ctx);

    REQUIRE(commands.cast_ability.empty());
}

TEST_CASE("trigger of unknown ability is silently ignored later", "[trigger]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;
    std::vector<DamageEvent> damage_events;

    AbilityDefinition fire_def{.name = "Fireball", .effects = {}};
    fire_def.effects.emplace_back(TriggerEffect{
        .ability_to_trigger = 9999,
        .chance = 1.0f,
    });
    const AbilityId fireball = registry.register_ability(std::move(fire_def));

    auto caster = world.spawn(EntityKind::Player);
    auto target = world.spawn(EntityKind::Enemy);

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 0.0f, 0.0f, ctx);
    REQUIRE(commands.cast_ability.size() == 1);

    apply_commands(system, ctx, &damage_events);
    REQUIRE(commands.empty());
}