#include "sim/ability_definition.hpp"
#include "sim/effect.hpp"
#include "sim/input_command.hpp"
#include "sim/sim.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace sim;
using Catch::Matchers::WithinAbs;

namespace {

constexpr SimFloat Tolerance = 0.0001f;

AbilityId register_simple_fireball(AbilityRegistry& registry) {
    AbilityDefinition def{
        .name = "Fireball",
        .tags = tags::Fire | tags::Spell,
        .effects = {},
    };
    def.effects.push_back(DamageEffect{
        .damage_stat = StatId::FireDamage,
        .base_amount = 50.0f,
    });
    return registry.register_ability(std::move(def));
}

}  // namespace

TEST_CASE("default sim starts at tick 0", "[sim]") {
    Sim sim;
    REQUIRE(sim.current_tick() == 0);
}

TEST_CASE("tick advances tick counter", "[sim]") {
    Sim sim;
    sim.tick();
    REQUIRE(sim.current_tick() == 1);
    sim.tick();
    sim.tick();
    REQUIRE(sim.current_tick() == 3);
}

TEST_CASE("advance_to runs ticks until target", "[sim]") {
    Sim sim;
    sim.advance_to(10);
    REQUIRE(sim.current_tick() == 10);
}

TEST_CASE("advance_to in past does nothing", "[sim]") {
    Sim sim;
    sim.advance_to(5);
    sim.advance_to(3);
    REQUIRE(sim.current_tick() == 5);
}

TEST_CASE("input cast command resolves through sim", "[sim]") {
    Sim sim;
    const AbilityId fireball = register_simple_fireball(sim.registry());

    auto caster = sim.world().spawn(EntityKind::Player);
    auto target = sim.world().spawn(EntityKind::Enemy);
    sim.world().health(target) = Health{.current = 100.0f, .max = 100.0f};

    sim.submit_input(InputCastCommand{
        .caster = caster,
        .ability_id = fireball,
        .target = target,
    });

    sim.tick();

    REQUIRE_THAT(sim.world().health(target).current, WithinAbs(50.0f, Tolerance));
}

TEST_CASE("input commands accumulate and process in tick order", "[sim]") {
    Sim sim;
    const AbilityId fireball = register_simple_fireball(sim.registry());

    auto caster = sim.world().spawn(EntityKind::Player);
    auto target = sim.world().spawn(EntityKind::Enemy);
    sim.world().health(target) = Health{.current = 200.0f, .max = 200.0f};

    sim.submit_input(InputCastCommand{
        .caster = caster,
        .ability_id = fireball,
        .target = target,
    });
    sim.submit_input(InputCastCommand{
        .caster = caster,
        .ability_id = fireball,
        .target = target,
    });

    sim.tick();

    REQUIRE_THAT(sim.world().health(target).current, WithinAbs(100.0f, Tolerance));
}

TEST_CASE("delayed cast resolves after correct number of ticks", "[sim]") {
    Sim sim;

    AbilityDefinition def{
        .name = "SlowFireball",
        .tags = tags::Fire | tags::Spell,
        .cast_time_ticks = 3,
        .effects = {},
    };
    def.effects.push_back(DamageEffect{
        .damage_stat = StatId::FireDamage,
        .base_amount = 50.0f,
    });
    const AbilityId fireball = sim.registry().register_ability(std::move(def));

    auto caster = sim.world().spawn(EntityKind::Player);
    auto target = sim.world().spawn(EntityKind::Enemy);
    sim.world().health(target) = Health{.current = 100.0f, .max = 100.0f};

    sim.submit_input(InputCastCommand{
        .caster = caster,
        .ability_id = fireball,
        .target = target,
    });

    sim.tick();
    REQUIRE(sim.world().health(target).current == 100.0f);

    sim.tick();
    REQUIRE(sim.world().health(target).current == 100.0f);

    sim.tick();
    REQUIRE(sim.world().health(target).current == 100.0f);

    sim.tick();
    REQUIRE_THAT(sim.world().health(target).current, WithinAbs(50.0f, Tolerance));
}

TEST_CASE("status DoT ticks across multiple sim ticks", "[sim]") {
    Sim sim;

    auto target = sim.world().spawn(EntityKind::Enemy);
    auto applier = sim.world().spawn(EntityKind::Player);
    sim.world().health(target) = Health{.current = 100.0f, .max = 100.0f};

    StatusInstance ignite{
        .type = StatusType::Ignite,
        .source = make_source_id(source_categories::Status, 1),
        .applier = applier,
        .apply_tick = 0,
        .expire_tick = 200,
        .last_tick_processed = 0,
        .payload = IgniteData{.damage_per_tick = 5.0f},
    };
    sim.world().status_list(target).add(ignite);

    sim.advance_to(31);

    REQUIRE_THAT(sim.world().health(target).current, WithinAbs(95.0f, Tolerance));

    sim.advance_to(61);
    REQUIRE_THAT(sim.world().health(target).current, WithinAbs(90.0f, Tolerance));
}

TEST_CASE("two sims with same seed produce same RNG output", "[sim][determinism]") {
    Sim sim_a(42);
    Sim sim_b(42);

    for (int i = 0; i < 100; ++i) {
        REQUIRE(sim_a.rng().next_u64() == sim_b.rng().next_u64());
    }
}

TEST_CASE("two sims with different seeds diverge", "[sim][determinism]") {
    Sim sim_a(1);
    Sim sim_b(2);

    REQUIRE(sim_a.rng().next_u64() != sim_b.rng().next_u64());
}

TEST_CASE("rng float output is in unit range", "[sim][rng]") {
    Sim sim(123);

    for (int i = 0; i < 1000; ++i) {
        const SimFloat v = sim.rng().next_float_unit();
        REQUIRE(v >= 0.0f);
        REQUIRE(v < 1.0f);
    }
}

TEST_CASE("identical input sequences produce identical outcomes", "[sim][determinism]") {
    auto run_scenario = [](uint64_t seed) {
        Sim sim(seed);
        const AbilityId fireball = register_simple_fireball(sim.registry());

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

        return sim.world().health(target).current;
    };

    const SimFloat result_a = run_scenario(42);
    const SimFloat result_b = run_scenario(42);
    REQUIRE(result_a == result_b);
}