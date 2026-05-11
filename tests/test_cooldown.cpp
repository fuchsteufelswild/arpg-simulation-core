#include "sim/ability_definition.hpp"
#include "sim/cooldown.hpp"
#include "sim/effect.hpp"
#include "sim/input_command.hpp"
#include "sim/sim.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace sim;

namespace {

AbilityId register_cd_fireball(AbilityRegistry& registry, uint32_t cooldown) {
    AbilityDefinition def{
        .name = "Fireball",
        .tags = tags::Fire | tags::Spell,
        .cast_time_ticks = 0,
        .cooldown_ticks = cooldown,
        .effects = {},
    };
    def.effects.emplace_back(DamageEffect{
        .damage_stat = StatId::FireDamage,
        .base_amount = 50.0f,
    });
    return registry.register_ability(std::move(def));
}

}  // namespace

TEST_CASE("ability with no cooldown can fire repeatedly", "[cooldown]") {
    Sim sim;
    const AbilityId fireball = register_cd_fireball(sim.ability_registry(), 0);

    auto caster = sim.world().spawn(EntityKind::Player);
    auto target = sim.world().spawn(EntityKind::Enemy);
    sim.world().health(target) = Health{.current = 1000.0f, .max = 1000.0f};

    for (int i = 0; i < 5; ++i) {
        sim.submit_input(InputCastCommand{
            .caster = caster,
            .ability_id = fireball,
            .target = target,
        });
        sim.tick();
    }

    REQUIRE(sim.world().health(target).current == 750.0f);
}

TEST_CASE("ability on cooldown is blocked until ready", "[cooldown]") {
    Sim sim;
    const AbilityId fireball = register_cd_fireball(sim.ability_registry(), 30);

    auto caster = sim.world().spawn(EntityKind::Player);
    auto target = sim.world().spawn(EntityKind::Enemy);
    sim.world().health(target) = Health{.current = 1000.0f, .max = 1000.0f};

    sim.submit_input(InputCastCommand{
        .caster = caster,
        .ability_id = fireball,
        .target = target,
    });
    sim.tick();

    REQUIRE(sim.world().health(target).current == 950.0f);

    sim.submit_input(InputCastCommand{
        .caster = caster,
        .ability_id = fireball,
        .target = target,
    });
    sim.tick();

    REQUIRE(sim.world().health(target).current == 950.0f);

    sim.advance_to(31);

    sim.submit_input(InputCastCommand{
        .caster = caster,
        .ability_id = fireball,
        .target = target,
    });
    sim.tick();

    REQUIRE(sim.world().health(target).current == 900.0f);
}

TEST_CASE("cooldown is per-caster", "[cooldown]") {
    Sim sim;
    const AbilityId fireball = register_cd_fireball(sim.ability_registry(), 30);

    auto caster_a = sim.world().spawn(EntityKind::Player);
    auto caster_b = sim.world().spawn(EntityKind::Player);
    auto target = sim.world().spawn(EntityKind::Enemy);
    sim.world().health(target) = Health{.current = 1000.0f, .max = 1000.0f};

    sim.submit_input(InputCastCommand{
        .caster = caster_a,
        .ability_id = fireball,
        .target = target,
    });
    sim.submit_input(InputCastCommand{
        .caster = caster_b,
        .ability_id = fireball,
        .target = target,
    });
    sim.tick();

    REQUIRE(sim.world().health(target).current == 900.0f);
}

TEST_CASE("ready_at returns the cooldown end tick", "[cooldown]") {
    CooldownList cooldowns;
    cooldowns.set(1, 100);
    REQUIRE(cooldowns.ready_at(1) == 100);
    REQUIRE(cooldowns.ready_at(2) == 0);
}

TEST_CASE("is_ready transitions to true at the right tick", "[cooldown]") {
    CooldownList cooldowns;
    cooldowns.set(1, 100);

    REQUIRE_FALSE(cooldowns.is_ready(1, 99));
    REQUIRE(cooldowns.is_ready(1, 100));
    REQUIRE(cooldowns.is_ready(1, 200));
}

TEST_CASE("set updates existing entry", "[cooldown]") {
    CooldownList cooldowns;
    cooldowns.set(1, 100);
    cooldowns.set(1, 50);
    REQUIRE(cooldowns.size() == 1);
    REQUIRE(cooldowns.ready_at(1) == 50);
}