#include "sim/ability_definition.hpp"
#include "sim/ability_registry.hpp"
#include "sim/ability_system.hpp"
#include "sim/damage.hpp"
#include "sim/damage_event.hpp"
#include "sim/effect_context.hpp"
#include "sim/sim_commands.hpp"
#include "sim/world.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace sim;
using Catch::Matchers::WithinAbs;

namespace {

constexpr SimFloat Tolerance = 0.0001f;

AbilityId
register_fireball(AbilityRegistry& registry, uint32_t cast_time = 0, SimFloat base_dmg = 40.0f) {
    AbilityDefinition def{
        .name = "Fireball",
        .tags = tags::Fire | tags::Spell | tags::Projectile,
        .cast_time_ticks = cast_time,
        .effects = {},
    };
    def.effects.push_back(DamageEffect{
        .damage_stat = StatId::FireDamage,
        .base_amount = base_dmg,
    });
    return registry.register_ability(std::move(def));
}

void apply_all_commands(SimCommands& commands, World& world) {
    SimCommands followup;
    std::vector<DamageEvent> damage_events;
    for (const auto& cmd : commands.deal_damage) {
        apply_damage(cmd, world, followup, &damage_events);
    }
    for (const auto& cmd : commands.kill_entity) {
        if (world.is_alive(cmd.entity)) {
            world.kill(cmd.entity);
        }
    }
    commands.clear();

    for (const auto& cmd : followup.kill_entity) {
        if (world.is_alive(cmd.entity)) {
            world.kill(cmd.entity);
        }
    }
}

}  // namespace

TEST_CASE("instant cast resolves immediately", "[ability_exec]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_fireball(registry);

    auto caster = world.spawn(EntityKind::Player);
    auto target = world.spawn(EntityKind::Enemy);
    world.health(target) = Health{.current = 100.0f, .max = 100.0f};

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 0.0f, 0.0f, ctx);
    REQUIRE(system.total_active() == 0);

    REQUIRE(commands.deal_damage.size() == 1);
    REQUIRE(commands.deal_damage[0].amount == 40.0f);
}

TEST_CASE("delayed cast resolves after cast time", "[ability_exec]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_fireball(registry, 15);

    auto caster = world.spawn(EntityKind::Player);
    auto target = world.spawn(EntityKind::Enemy);
    world.health(target) = Health{.current = 100.0f, .max = 100.0f};

    ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 0.0f, 0.0f, ctx);
    REQUIRE(system.casting().size() == 1);

    ctx.current_tick = 5;
    system.update(ctx);
    REQUIRE(system.casting().size() == 1);
    REQUIRE(commands.deal_damage.empty());

    ctx.current_tick = 15;
    system.update(ctx);
    REQUIRE(system.casting().empty());
    REQUIRE(commands.deal_damage.size() == 1);
}

TEST_CASE("damage is queued, not applied directly", "[ability_exec]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_fireball(registry);

    auto caster = world.spawn(EntityKind::Player);
    auto target = world.spawn(EntityKind::Enemy);
    world.health(target) = Health{.current = 100.0f, .max = 100.0f};

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 0.0f, 0.0f, ctx);

    REQUIRE(world.health(target).current == 100.0f);
    REQUIRE(commands.deal_damage.size() == 1);

    apply_all_commands(commands, world);
    REQUIRE_THAT(world.health(target).current, WithinAbs(60.0f, Tolerance));
}

TEST_CASE("damage scales with caster modifiers", "[ability_exec]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_fireball(registry, 0, 40.0f);

    auto caster = world.spawn(EntityKind::Player);
    auto target = world.spawn(EntityKind::Enemy);
    world.health(target) = Health{.current = 200.0f, .max = 200.0f};

    world.stats(caster).add_modifier(Modifier{
        .stat = StatId::FireDamage,
        .op = ModOp::Increased,
        .magnitude = 0.50f,
        .source = make_source_id(source_categories::Gear, 1),
    });

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 0.0f, 0.0f, ctx);

    REQUIRE(commands.deal_damage.size() == 1);
    REQUIRE_THAT(commands.deal_damage[0].amount, WithinAbs(40.0f, Tolerance));
}

TEST_CASE("damage queued from caster modifier of damage stat", "[ability_exec]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    AbilityDefinition def{
        .name = "Fireball",
        .tags = tags::Fire | tags::Spell,
        .effects = {},
    };
    def.effects.emplace_back(DamageEffect{
        .damage_stat = StatId::FireDamage,
        .base_amount = 0.0f,
    });
    const AbilityId fireball = registry.register_ability(std::move(def));

    auto caster = world.spawn(EntityKind::Player);
    auto target = world.spawn(EntityKind::Enemy);
    world.health(target) = Health{.current = 200.0f, .max = 200.0f};

    world.stats(caster).add_modifier(Modifier{
        .stat = StatId::FireDamage,
        .op = ModOp::Flat,
        .magnitude = 50.0f,
        .source = make_source_id(source_categories::Gear, 1),
    });
    world.stats(caster).add_modifier(Modifier{
        .stat = StatId::FireDamage,
        .op = ModOp::Increased,
        .magnitude = 0.50f,
        .source = make_source_id(source_categories::Gear, 2),
    });

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 0.0f, 0.0f, ctx);

    REQUIRE(commands.deal_damage.size() == 1);
    REQUIRE_THAT(commands.deal_damage[0].amount, WithinAbs(75.0f, Tolerance));
}

TEST_CASE("dead target before resolution: damage skipped", "[ability_exec]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_fireball(registry, 15);

    auto caster = world.spawn(EntityKind::Player);
    auto target = world.spawn(EntityKind::Enemy);
    world.health(target) = Health{.current = 100.0f, .max = 100.0f};

    ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 0.0f, 0.0f, ctx);
    world.kill(target);

    ctx.current_tick = 15;
    system.update(ctx);
    REQUIRE(commands.deal_damage.empty());
}

TEST_CASE("damage to zero queues kill command", "[ability_exec]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_fireball(registry, 0, 200.0f);

    auto caster = world.spawn(EntityKind::Player);
    auto target = world.spawn(EntityKind::Enemy);
    world.health(target) = Health{.current = 100.0f, .max = 100.0f};

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 0.0f, 0.0f, ctx);
    apply_all_commands(commands, world);

    REQUIRE_FALSE(world.is_alive(target));
}

TEST_CASE("two damage events same tick both apply", "[ability_exec]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_fireball(registry, 0, 30.0f);

    auto caster = world.spawn(EntityKind::Player);
    auto target = world.spawn(EntityKind::Enemy);
    world.health(target) = Health{.current = 100.0f, .max = 100.0f};

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 0.0f, 0.0f, ctx);
    system.cast(fireball, caster, target, 0.0f, 0.0f, ctx);

    REQUIRE(commands.deal_damage.size() == 2);
    apply_all_commands(commands, world);
    REQUIRE_THAT(world.health(target).current, WithinAbs(40.0f, Tolerance));
}

TEST_CASE("unknown ability id silently does nothing", "[ability_exec]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    auto caster = world.spawn(EntityKind::Player);
    auto target = world.spawn(EntityKind::Enemy);

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(999, caster, target, 0.0f, 0.0f, ctx);

    REQUIRE(system.total_active() == 0);
    REQUIRE(commands.empty());
}

TEST_CASE("dead caster cannot cast", "[ability_exec]") {
    AbilityRegistry registry;
    AbilitySystem system;
    World world;
    SimCommands commands;
    SpatialGrid grid;

    const AbilityId fireball = register_fireball(registry);

    auto caster = world.spawn(EntityKind::Player);
    auto target = world.spawn(EntityKind::Enemy);
    world.kill(caster);

    const ResolutionContext ctx{
        .world = &world,
        .registry = &registry,
        .grid = &grid,
        .commands = &commands,
        .current_tick = 0,
    };

    system.cast(fireball, caster, target, 0.0f, 0.0f, ctx);

    REQUIRE(commands.empty());
}