#include "sim/ai_behaviour.hpp"
#include "sim/data/bootstrap.hpp"
#include "sim/entity_archetype_registry.hpp"
#include "sim/sim.hpp"
#include "sim/world.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace sim;
using Catch::Matchers::WithinAbs;

namespace {
constexpr SimFloat Tolerance = 0.001f;

constexpr std::string_view ArchetypeAbilitiesToml = R"(
[[ability]]
name = "Fireball"
[[ability.effects]]
type = "Damage"
damage_stat = "FireDamage"
base_amount = 40.0
)";

constexpr std::string_view ArchetypesToml = R"(
[[archetype]]
name = "Goblin"
kind = "Enemy"
max_health = 80.0
[archetype.ai]
preferred_ability = "Fireball"
attack_range = 10.0
detection_range = 20.0
move_speed = 0.4
 
[[archetype]]
name = "Player"
kind = "Player"
max_health = 500.0
)";

AbilityId register_simple_attack(AbilityRegistry& registry) {
    AbilityDefinition def{.name = "Strike", .tags = tags::Attack, .effects = {}};
    def.effects.emplace_back(DamageEffect{
        .damage_stat = StatId::PhysicalDamage,
        .base_amount = 100.0f,
    });
    return registry.register_ability(std::move(def));
}

EntityHandle spawn_ai_enemy(Sim& sim,
                            AbilityId ability,
                            SimFloat x,
                            SimFloat y,
                            SimFloat attack_range,
                            SimFloat detection_range,
                            SimFloat move_speed) {
    auto handle = sim.world().spawn(EntityKind::Enemy);
    sim.world().transform(handle) = Transform{.x = x, .y = y, .facing_radians = 0.0f};
    sim.world().health(handle) = Health{.current = 100.0f, .max = 100.0f};
    sim.world().ai_behaviour(handle) = AIBehaviour{
        .state = AIState::Idle,
        .preferred_ability = ability,
        .attack_range = attack_range,
        .detection_range = detection_range,
        .leash_range = 0.0f,
        .move_speed = move_speed,
    };
    return handle;
}

EntityHandle spawn_player(Sim& sim) {
    const EntityHandle player = sim.world().spawn(EntityKind::Player);
    sim.world().transform(player) = Transform{.x = 0.0f, .y = 0.0f, .facing_radians = 0.0f};
    sim.world().health(player) = Health{.current = 100.0f, .max = 100.0f};
    return player;
}
}  // namespace

TEST_CASE("AI enemy stays idle when player is out of detection range", "[ai]") {
    Sim sim(42);
    const AbilityId strike = register_simple_attack(sim.ability_registry());

    auto enemy = spawn_ai_enemy(sim, strike, 100.0f, 0.0f, 2.0f, 10.0f, 0.5f);

    sim.advance_to(30);

    REQUIRE(sim.world().ai_behaviour(enemy).state == AIState::Idle);
    REQUIRE(sim.world().ai_behaviour(enemy).target.is_null());
    REQUIRE_THAT(sim.world().transform(enemy).x, WithinAbs(100.0f, Tolerance));
}

TEST_CASE("AI enemy chases player when in detection range but out of attack range", "[ai]") {
    Sim sim(42);
    const AbilityId strike = register_simple_attack(sim.ability_registry());

    auto enemy = spawn_ai_enemy(sim, strike, 5.0f, 0.0f, 1.0f, 10.0f, 0.5f);
    auto player = spawn_player(sim);

    sim.tick();

    const SimFloat start_x = sim.world().transform(enemy).x;

    sim.tick();

    REQUIRE(sim.world().ai_behaviour(enemy).state == AIState::Chase);
    REQUIRE(!sim.world().ai_behaviour(enemy).target.is_null());
    REQUIRE(sim.world().transform(enemy).x < start_x);
}

TEST_CASE("AI enemy attacks player when in range", "[ai]") {
    Sim sim(42);
    const AbilityId strike = register_simple_attack(sim.ability_registry());

    auto enemy = spawn_ai_enemy(sim, strike, 1.5f, 0.0f, 2.0f, 10.0f, 0.5f);
    sim.world().health(enemy) = Health{.current = 100.0f, .max = 100.0f};

    const EntityHandle player = spawn_player(sim);
    REQUIRE(!player.is_null());
    sim.world().health(player) = Health{.current = 400.0f, .max = 400.0f};

    sim.tick();
    REQUIRE(sim.world().ai_behaviour(enemy).state == AIState::Attack);
    sim.advance_to(31);

    REQUIRE_THAT(sim.world().transform(enemy).x, WithinAbs(1.5f, Tolerance));
    REQUIRE_THAT(sim.world().health(player).current, WithinAbs(100.0f, Tolerance));
}

TEST_CASE("AI enemy returns to idle when its target dies", "[ai]") {
    Sim sim(42);
    const AbilityId strike = register_simple_attack(sim.ability_registry());

    auto enemy = spawn_ai_enemy(sim, strike, 1.5f, 0.0f, 2.0f, 10.0f, 0.5f);
    auto player = spawn_player(sim);

    sim.tick();
    REQUIRE(sim.world().ai_behaviour(enemy).state == AIState::Attack);

    REQUIRE(!player.is_null());
    sim.world().kill(player);

    sim.tick();

    REQUIRE(sim.world().ai_behaviour(enemy).state == AIState::Idle);
    REQUIRE(sim.world().ai_behaviour(enemy).target.is_null());
}

TEST_CASE("AI with state Inactive is never ticked", "[ai]") {
    Sim sim(42);
    [[maybe_unused]] const AbilityId strike = register_simple_attack(sim.ability_registry());

    auto enemy = sim.world().spawn(EntityKind::Enemy);
    sim.world().transform(enemy) = Transform{.x = 1.5f, .y = 0.0f};
    sim.world().health(enemy) = Health{.current = 100.0f, .max = 100.0f};

    sim.advance_to(30);

    REQUIRE(sim.world().ai_behaviour(enemy).state == AIState::Inactive);
    REQUIRE(sim.world().ai_behaviour(enemy).target.is_null());
    REQUIRE_THAT(sim.world().transform(enemy).x, WithinAbs(1.5f, Tolerance));
}

TEST_CASE("Archetype-spawned enemy acquires player target on tick", "[ai][archetype]") {
    Sim sim(42);

    REQUIRE(sim::data::bootstrap_from_strings(sim,
                                              sim::data::ContentSources{
                                                  .abilities_toml = ArchetypeAbilitiesToml,
                                                  .archetypes_toml = ArchetypesToml,
                                              })
                .has_value());

    auto player_result = sim::data::spawn_archetype_by_name(sim, "Player", 0.0f, 0.0f);
    REQUIRE(player_result.has_value());
    const EntityHandle player = *player_result;

    const EntityHandle goblin = sim.archetype_registry().spawn(
        sim.world(), sim.archetype_registry().find_by_name("Goblin"));
    sim.world().transform(goblin) = Transform{.x = 5.0f, .y = 0.0f};

    REQUIRE(sim.world().health(goblin).current == 80.0f);
    REQUIRE(sim.world().ai_behaviour(goblin).state == AIState::Idle);

    sim.tick();

    REQUIRE(!sim.world().ai_behaviour(goblin).target.is_null());
    REQUIRE(sim.world().ai_behaviour(goblin).target.index == player.index);
}

TEST_CASE("Archetype-spawned player has no AI", "[ai][archetype]") {
    Sim sim(42);

    REQUIRE(sim::data::bootstrap_from_strings(sim,
                                              sim::data::ContentSources{
                                                  .abilities_toml = ArchetypeAbilitiesToml,
                                                  .archetypes_toml = ArchetypesToml,
                                              })
                .has_value());

    auto player_result = sim::data::spawn_archetype_by_name(sim, "Player", 0.0f, 0.0f);
    REQUIRE(player_result.has_value());
    const EntityHandle player = *player_result;

    REQUIRE(sim.world().kind(player) == EntityKind::Player);
    REQUIRE(sim.world().health(player).current == 500.0f);
    REQUIRE(sim.world().ai_behaviour(player).state == AIState::Inactive);
}