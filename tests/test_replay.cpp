#include "sim/ability_definition.hpp"
#include "sim/effect.hpp"
#include "sim/input_log.hpp"
#include "sim/replay.hpp"
#include "sim/sim.hpp"
#include "sim/world_hash.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace sim;

namespace {

AbilityId register_replay_fireball(AbilityRegistry& registry) {
    AbilityDefinition def{
        .name = "Fireball",
        .tags = tags::Fire | tags::Spell,
        .effects = {},
    };
    def.effects.emplace_back(DamageEffect{
        .damage_stat = StatId::FireDamage,
        .base_amount = 30.0f,
    });
    return registry.register_ability(std::move(def));
}

void setup_simple_scenario(Sim& sim) {
    register_replay_fireball(sim.registry());
    auto caster = sim.world().spawn(EntityKind::Player);
    sim.world().stats(caster).add_modifier(Modifier{
        .stat = StatId::CritChance,
        .op = ModOp::Flat,
        .magnitude = 0.20f,
        .source = make_source_id(source_categories::Gear, 1),
    });
    auto target = sim.world().spawn(EntityKind::Enemy);
    sim.world().health(target) = Health{.current = 5000.0f, .max = 5000.0f};
}

}  // namespace

TEST_CASE("empty log produces consistent hash", "[replay]") {
    const InputLog log;

    const auto result_a = replay(log, 42, 100, setup_simple_scenario);
    const auto result_b = replay(log, 42, 100, setup_simple_scenario);

    REQUIRE(result_a.final_hash == result_b.final_hash);
    REQUIRE(result_a.ticks_run == 100);
}

TEST_CASE("different seeds produce different hashes", "[replay]") {
    const InputLog log;

    const auto result_a = replay(log, 1, 50, setup_simple_scenario);
    const auto result_b = replay(log, 2, 50, setup_simple_scenario);

    REQUIRE(result_a.final_hash != result_b.final_hash);
}

TEST_CASE("recorded inputs reproduce identical state", "[replay]") {
    InputLog log;

    const EntityHandle caster{.index = 0, .generation = 1};
    const EntityHandle target{.index = 1, .generation = 1};

    log.record(0,
               InputCastCommand{
                   .caster = caster,
                   .ability_id = 1,
                   .target = target,
               });
    log.record(5,
               InputCastCommand{
                   .caster = caster,
                   .ability_id = 1,
                   .target = target,
               });
    log.record(10,
               InputCastCommand{
                   .caster = caster,
                   .ability_id = 1,
                   .target = target,
               });

    const auto result_a = replay(log, 42, 30, setup_simple_scenario);
    const auto result_b = replay(log, 42, 30, setup_simple_scenario);

    REQUIRE(result_a.final_hash == result_b.final_hash);
}

TEST_CASE("input log roundtrips through serialization", "[replay][serialization]") {
    InputLog original;
    original.record(5,
                    InputCastCommand{
                        .caster = EntityHandle{.index = 1, .generation = 2},
                        .ability_id = 7,
                        .target = EntityHandle{.index = 3, .generation = 4},
                        .target_x = 12.5f,
                        .target_y = -8.25f,
                    });
    original.record(10,
                    InputMoveCommand{
                        .entity = EntityHandle{.index = 1, .generation = 2},
                        .direction_x = 0.707f,
                        .direction_y = 0.707f,
                    });

    const auto bytes = original.serialize();
    const auto restored = InputLog::deserialize(bytes);

    REQUIRE(restored.size() == 2);
    REQUIRE(restored.entries()[0].submitted_at_tick == 5);
    REQUIRE(restored.entries()[1].submitted_at_tick == 10);
}

TEST_CASE("serialized log replays to same hash", "[replay][serialization]") {
    InputLog original;
    original.record(0,
                    InputCastCommand{
                        .caster = EntityHandle{.index = 0, .generation = 1},
                        .ability_id = 1,
                        .target = EntityHandle{.index = 1, .generation = 1},
                    });
    original.record(15,
                    InputCastCommand{
                        .caster = EntityHandle{.index = 0, .generation = 1},
                        .ability_id = 1,
                        .target = EntityHandle{.index = 1, .generation = 1},
                    });

    const auto bytes = original.serialize();
    const auto restored = InputLog::deserialize(bytes);

    const auto result_original = replay(original, 42, 50, setup_simple_scenario);
    const auto result_restored = replay(restored, 42, 50, setup_simple_scenario);

    REQUIRE(result_original.final_hash == result_restored.final_hash);
}

TEST_CASE("invalid magic returns empty log", "[replay][serialization]") {
    std::vector<uint8_t> bad_bytes(32, 0xFF);
    const auto log = InputLog::deserialize(bad_bytes);
    REQUIRE(log.size() == 0);
}

TEST_CASE("hash detects state divergence", "[replay][hash]") {
    Sim sim_a(42);
    Sim sim_b(42);

    setup_simple_scenario(sim_a);
    setup_simple_scenario(sim_b);

    REQUIRE(hash_world_state(sim_a) == hash_world_state(sim_b));

    auto extra = sim_b.world().spawn(EntityKind::Enemy);
    sim_b.world().health(extra) = Health{.current = 100.0f, .max = 100.0f};

    REQUIRE(hash_world_state(sim_a) != hash_world_state(sim_b));
}

TEST_CASE("complex scenario with crits replays deterministically", "[replay][determinism]") {
    InputLog log;
    const EntityHandle caster{.index = 0, .generation = 1};
    const EntityHandle target{.index = 1, .generation = 1};

    for (uint64_t i = 0; i < 20; ++i) {
        log.record(i * 3,
                   InputCastCommand{
                       .caster = caster,
                       .ability_id = 1,
                       .target = target,
                   });
    }

    auto setup = [](Sim& sim) {
        register_replay_fireball(sim.registry());
        auto c = sim.world().spawn(EntityKind::Player);
        sim.world().stats(c).add_modifier(Modifier{
            .stat = StatId::CritChance,
            .op = ModOp::Flat,
            .magnitude = 0.50f,
            .source = make_source_id(source_categories::Gear, 1),
        });
        auto t = sim.world().spawn(EntityKind::Enemy);
        sim.world().health(t) = Health{.current = 100000.0f, .max = 100000.0f};
    };

    const auto result_a = replay(log, 12345, 100, setup);
    const auto result_b = replay(log, 12345, 100, setup);
    const auto result_c = replay(log, 99999, 100, setup);

    REQUIRE(result_a.final_hash == result_b.final_hash);
    REQUIRE(result_a.final_hash != result_c.final_hash);
}