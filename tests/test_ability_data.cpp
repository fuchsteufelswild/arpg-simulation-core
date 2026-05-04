#include "sim/ability_definition.hpp"
#include "sim/ability_instance.hpp"
#include "sim/ability_registry.hpp"
#include "sim/ability_system.hpp"
#include "sim/util/overload.hpp"

#include <catch2/catch_test_macros.hpp>

#include <variant>

using namespace sim;

TEST_CASE("default ability definition is empty", "[ability]") {
    AbilityDefinition def;
    REQUIRE(def.id == NoAbility);
    REQUIRE(def.name.empty());
    REQUIRE(def.tags == tags::None);
    REQUIRE(def.cast_time_ticks == 0);
    REQUIRE(def.travel_speed == 0.0f);
    REQUIRE(def.effects.empty());
}

TEST_CASE("ability definition holds effects in order", "[ability]") {
    AbilityDefinition def{
        .name = "Fireball",
        .tags = tags::Fire | tags::Spell | tags::Projectile,
        .cast_time_ticks = 15,
        .effects = {},
    };
    def.effects.push_back(DamageEffect{.damage_stat = StatId::FireDamage, .base_amount = 40.0f});
    def.effects.push_back(ChainEffect{.max_chains = 3, .falloff = 0.8f, .radius = 5.0f});

    REQUIRE(def.effects.size() == 2);
    REQUIRE(std::holds_alternative<DamageEffect>(def.effects[0]));
    REQUIRE(std::holds_alternative<ChainEffect>(def.effects[1]));

    const auto& dmg = std::get<DamageEffect>(def.effects[0]);
    REQUIRE(dmg.damage_stat == StatId::FireDamage);
    REQUIRE(dmg.base_amount == 40.0f);
}

TEST_CASE("registry assigns IDs starting at 1", "[ability]") {
    AbilityRegistry registry;
    const auto id1 = registry.register_ability({.name = "Fireball", .effects = {}});
    const auto id2 = registry.register_ability({.name = "IceNova", .effects = {}});

    REQUIRE(id1 != NoAbility);
    REQUIRE(id2 != NoAbility);
    REQUIRE(id1 != id2);
    REQUIRE(registry.count() == 2);
}

TEST_CASE("registry lookup returns matching definition", "[ability]") {
    AbilityRegistry registry;
    const auto id =
        registry.register_ability({.name = "Fireball", .cast_time_ticks = 15, .effects = {}});

    const auto* def = registry.find(id);
    REQUIRE(def != nullptr);
    REQUIRE(def->id == id);
    REQUIRE(def->name == "Fireball");
    REQUIRE(def->cast_time_ticks == 15);
}

TEST_CASE("registry lookup of unknown ID returns nullptr", "[ability]") {
    AbilityRegistry registry;
    REQUIRE(registry.find(NoAbility) == nullptr);
    REQUIRE(registry.find(999) == nullptr);
}

TEST_CASE("registry clear empties storage", "[ability]") {
    AbilityRegistry registry;
    registry.register_ability({.name = "Fireball", .effects = {}});
    registry.register_ability({.name = "IceNova", .effects = {}});
    registry.clear();

    REQUIRE(registry.count() == 0);
}

TEST_CASE("ability system starts empty", "[ability_system]") {
    AbilitySystem system;
    REQUIRE(system.total_active() == 0);
    REQUIRE(system.casting().empty());
    REQUIRE(system.traveling().empty());
    REQUIRE(system.resolving().empty());
}

TEST_CASE("add routes instance to correct phase list", "[ability_system]") {
    AbilitySystem system;

    system.add(AbilityInstance{.phase = AbilityPhase::Casting, .caster = {}, .initial_target = {}});
    system.add(AbilityInstance{.phase = AbilityPhase::Casting, .caster = {}, .initial_target = {}});
    system.add(
        AbilityInstance{.phase = AbilityPhase::Traveling, .caster = {}, .initial_target = {}});
    system.add(
        AbilityInstance{.phase = AbilityPhase::Resolving, .caster = {}, .initial_target = {}});

    REQUIRE(system.casting().size() == 2);
    REQUIRE(system.traveling().size() == 1);
    REQUIRE(system.resolving().size() == 1);
    REQUIRE(system.total_active() == 4);
}

TEST_CASE("finished instance is silently dropped", "[ability_system]") {
    AbilitySystem system;
    system.add(
        AbilityInstance{.phase = AbilityPhase::Finished, .caster = {}, .initial_target = {}});

    REQUIRE(system.total_active() == 0);
}

TEST_CASE("clear empties all phase lists", "[ability_system]") {
    AbilitySystem system;
    system.add(AbilityInstance{.phase = AbilityPhase::Casting, .caster = {}, .initial_target = {}});
    system.add(
        AbilityInstance{.phase = AbilityPhase::Traveling, .caster = {}, .initial_target = {}});
    system.clear();

    REQUIRE(system.total_active() == 0);
}

TEST_CASE("overload pattern dispatches on variant alternatives", "[util]") {
    Effect effect = DamageEffect{.damage_stat = StatId::FireDamage, .base_amount = 50.0f};

    int matched = 0;
    SimFloat captured = 0.0f;

    std::visit(util::Overload{
                   [&](const DamageEffect& d) {
                       matched = 1;
                       captured = d.base_amount;
                   },
                   [&](const ApplyStatusEffect&) { matched = 2; },
                   [&](const ChainEffect&) { matched = 3; },
                   [&](const TriggerEffect&) { matched = 4; },
               },
               effect);

    REQUIRE(matched == 1);
    REQUIRE(captured == 50.0f);
}