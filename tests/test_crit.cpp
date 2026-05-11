#include "sim/ability_definition.hpp"
#include "sim/effect.hpp"
#include "sim/input_command.hpp"
#include "sim/sim.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace sim;
using Catch::Matchers::WithinAbs;

namespace {

constexpr SimFloat Tolerance = 0.01f;

AbilityId register_simple_attack(AbilityRegistry& registry) {
    AbilityDefinition def{.name = "Strike", .tags = tags::Attack, .effects = {}};
    def.effects.emplace_back(DamageEffect{
        .damage_stat = StatId::PhysicalDamage,
        .base_amount = 100.0f,
    });
    return registry.register_ability(std::move(def));
}

}  // namespace

TEST_CASE("zero crit chance never crits", "[crit]") {
    Sim sim(42);
    const AbilityId strike = register_simple_attack(sim.ability_registry());

    auto caster = sim.world().spawn(EntityKind::Player);
    auto target = sim.world().spawn(EntityKind::Enemy);
    sim.world().health(target) = Health{.current = 10000.0f, .max = 10000.0f};

    for (int i = 0; i < 50; ++i) {
        sim.submit_input(InputCastCommand{
            .caster = caster,
            .ability_id = strike,
            .target = target,
        });
        sim.tick();
    }

    REQUIRE(sim.world().health(target).current == 5000.0f);
}

TEST_CASE("100% crit chance always crits", "[crit]") {
    Sim sim(42);
    const AbilityId strike = register_simple_attack(sim.ability_registry());

    auto caster = sim.world().spawn(EntityKind::Player);
    sim.world().stats(caster).add_modifier(Modifier{
        .stat = StatId::CritChance,
        .op = ModOp::Flat,
        .magnitude = 1.0f,
        .source = make_source_id(source_categories::Gear, 1),
    });

    auto target = sim.world().spawn(EntityKind::Enemy);
    sim.world().health(target) = Health{.current = 1000.0f, .max = 1000.0f};

    sim.submit_input(InputCastCommand{
        .caster = caster,
        .ability_id = strike,
        .target = target,
    });
    sim.tick();

    REQUIRE_THAT(sim.world().health(target).current, WithinAbs(850.0f, Tolerance));
}

TEST_CASE("crit multiplier scales the crit damage", "[crit]") {
    Sim sim(42);
    const AbilityId strike = register_simple_attack(sim.ability_registry());

    auto caster = sim.world().spawn(EntityKind::Player);
    sim.world().stats(caster).add_modifier(Modifier{
        .stat = StatId::CritChance,
        .op = ModOp::Flat,
        .magnitude = 1.0f,
        .source = make_source_id(source_categories::Gear, 1),
    });
    sim.world().stats(caster).add_modifier(Modifier{
        .stat = StatId::CritMultiplier,
        .op = ModOp::Flat,
        .magnitude = 2.0f,
        .source = make_source_id(source_categories::Gear, 2),
    });

    auto target = sim.world().spawn(EntityKind::Enemy);
    sim.world().health(target) = Health{.current = 1000.0f, .max = 1000.0f};

    sim.submit_input(InputCastCommand{
        .caster = caster,
        .ability_id = strike,
        .target = target,
    });
    sim.tick();

    REQUIRE_THAT(sim.world().health(target).current, WithinAbs(800.0f, Tolerance));
}

TEST_CASE("OnCrit modifier only applies on critical hits", "[crit][conditions]") {
    Sim sim(42);
    const AbilityId strike = register_simple_attack(sim.ability_registry());

    auto caster = sim.world().spawn(EntityKind::Player);
    sim.world().stats(caster).add_modifier(Modifier{
        .stat = StatId::CritChance,
        .op = ModOp::Flat,
        .magnitude = 1.0f,
        .source = make_source_id(source_categories::Gear, 1),
    });
    sim.world().stats(caster).add_modifier(Modifier{
        .stat = StatId::PhysicalDamage,
        .op = ModOp::Flat,
        .magnitude = 100.0f,
        .source = make_source_id(source_categories::Gear, 2),
    });
    sim.world().stats(caster).add_modifier(Modifier{
        .stat = StatId::PhysicalDamage,
        .op = ModOp::Increased,
        .condition = ConditionId::OnCrit,
        .magnitude = 1.0f,
        .source = make_source_id(source_categories::Gear, 3),
    });

    auto target = sim.world().spawn(EntityKind::Enemy);
    sim.world().health(target) = Health{.current = 1000.0f, .max = 1000.0f};

    sim.submit_input(InputCastCommand{
        .caster = caster,
        .ability_id = strike,
        .target = target,
    });
    sim.tick();

    REQUIRE_THAT(sim.world().health(target).current, WithinAbs(550.0f, Tolerance));
}

TEST_CASE("CritRecently condition picks up crit events", "[crit][conditions]") {
    Sim sim(42);
    const AbilityId strike = register_simple_attack(sim.ability_registry());

    auto caster = sim.world().spawn(EntityKind::Player);
    sim.world().stats(caster).add_modifier(Modifier{
        .stat = StatId::CritChance,
        .op = ModOp::Flat,
        .magnitude = 1.0f,
        .source = make_source_id(source_categories::Gear, 1),
    });

    auto target = sim.world().spawn(EntityKind::Enemy);
    sim.world().health(target) = Health{.current = 10000.0f, .max = 10000.0f};

    sim.submit_input(InputCastCommand{
        .caster = caster,
        .ability_id = strike,
        .target = target,
    });
    sim.tick();

    REQUIRE(
        sim.world().recent_events(caster).any_within(EventType::CritHit, sim.current_tick(), 100));
}

TEST_CASE("damage event records crit flag", "[crit]") {
    Sim sim(42);
    const AbilityId strike = register_simple_attack(sim.ability_registry());

    auto caster = sim.world().spawn(EntityKind::Player);
    sim.world().stats(caster).add_modifier(Modifier{
        .stat = StatId::CritChance,
        .op = ModOp::Flat,
        .magnitude = 1.0f,
        .source = make_source_id(source_categories::Gear, 1),
    });

    auto target = sim.world().spawn(EntityKind::Enemy);
    sim.world().health(target) = Health{.current = 1000.0f, .max = 1000.0f};

    sim.submit_input(InputCastCommand{
        .caster = caster,
        .ability_id = strike,
        .target = target,
    });
    sim.tick();

    const auto events = sim.damage_events();
    REQUIRE(events.size() == 1);
    REQUIRE(events[0].was_crit);
}

TEST_CASE("same seed produces same crit sequence", "[crit][determinism]") {
    auto run = [](uint64_t seed) -> std::vector<bool> {
        Sim sim(seed);
        const AbilityId strike = register_simple_attack(sim.ability_registry());

        auto caster = sim.world().spawn(EntityKind::Player);
        sim.world().stats(caster).add_modifier(Modifier{
            .stat = StatId::CritChance,
            .op = ModOp::Flat,
            .magnitude = 0.30f,
            .source = make_source_id(source_categories::Gear, 1),
        });

        auto target = sim.world().spawn(EntityKind::Enemy);
        sim.world().health(target) = Health{.current = 100000.0f, .max = 100000.0f};

        std::vector<bool> crits;
        for (int i = 0; i < 30; ++i) {
            sim.submit_input(InputCastCommand{
                .caster = caster,
                .ability_id = strike,
                .target = target,
            });
            sim.tick();
            const auto events = sim.damage_events();
            if (!events.empty()) {
                crits.push_back(events[0].was_crit);
            }
        }
        return crits;
    };

    const auto run_a = run(42);
    const auto run_b = run(42);
    REQUIRE(run_a == run_b);
}