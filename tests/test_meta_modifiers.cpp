#include "sim/entity_stats.hpp"
#include "sim/eval_context.hpp"
#include "sim/evaluate.hpp"
#include "sim/world.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace sim;
using Catch::Matchers::WithinAbs;

namespace {

constexpr SimFloat Tolerance = 0.0001f;

constexpr SourceId GearRing = make_source_id(source_categories::Gear, 1);
constexpr SourceId GearAmulet = make_source_id(source_categories::Gear, 2);
constexpr SourceId AuraSpell = make_source_id(source_categories::Aura, 1);

Modifier flat_fire_damage(SimFloat amount, SourceId source, TagMask required = tags::None) {
    return Modifier{
        .stat = StatId::FireDamage,
        .op = ModOp::Flat,
        .required_tags = required,
        .magnitude = amount,
        .source = source,
    };
}

Modifier increased_fire_damage(SimFloat fraction, SourceId source, TagMask required = tags::None) {
    return Modifier{
        .stat = StatId::FireDamage,
        .op = ModOp::Increased,
        .required_tags = required,
        .magnitude = fraction,
        .source = source,
    };
}

Modifier meta(TagMask required, SimFloat fraction, SourceId source) {
    return Modifier{
        .stat = StatId::None,
        .op = ModOp::Increased,
        .phase = ModPhase::Meta,
        .required_tags = required,
        .magnitude = fraction,
        .source = source,
    };
}

}  // namespace

TEST_CASE("meta modifier amplifies a matching primary increased", "[meta]") {
    World world;
    EntityStats stats;
    stats.add_modifier(flat_fire_damage(100.0f, GearRing));
    stats.add_modifier(increased_fire_damage(0.20f, GearAmulet, tags::Spell));
    stats.add_modifier(meta(tags::Spell, 0.50f, AuraSpell));

    EvalContext ctx{.ability_tags = tags::Spell | tags::Fire};

    REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                 WithinAbs(100.0f * 1.30f, Tolerance));
}

TEST_CASE("meta modifier does not amplify primary without matching tags", "[meta]") {
    World world;
    EntityStats stats;
    stats.add_modifier(flat_fire_damage(100.0f, GearRing));
    stats.add_modifier(increased_fire_damage(0.20f, GearAmulet));
    stats.add_modifier(meta(tags::Spell, 0.50f, AuraSpell));

    EvalContext ctx{.ability_tags = tags::Spell | tags::Fire};

    REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                 WithinAbs(100.0f * 1.20f, Tolerance));
}

TEST_CASE("meta modifier amplifies flat primary", "[meta]") {
    World world;
    EntityStats stats;
    stats.add_modifier(flat_fire_damage(100.0f, GearRing, tags::Spell));
    stats.add_modifier(meta(tags::Spell, 0.50f, AuraSpell));

    EvalContext ctx{.ability_tags = tags::Spell | tags::Fire};

    REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                 WithinAbs(150.0f, Tolerance));
}

TEST_CASE("multiple meta modifiers stack multiplicatively", "[meta]") {
    World world;
    EntityStats stats;
    stats.add_modifier(increased_fire_damage(0.20f, GearRing, tags::Spell));
    stats.add_modifier(flat_fire_damage(100.0f, GearAmulet, tags::Spell));
    stats.add_modifier(meta(tags::Spell, 0.50f, AuraSpell));
    stats.add_modifier(meta(tags::Spell, 0.20f, make_source_id(source_categories::Aura, 2)));

    EvalContext ctx{.ability_tags = tags::Spell | tags::Fire};

    const SimFloat expected_flat = 100.0f * 1.50f * 1.20f;
    const SimFloat expected_increased = 0.20f * 1.50f * 1.20f;
    const SimFloat expected = expected_flat * (1.0f + expected_increased);

    REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                 WithinAbs(expected, Tolerance));
}

TEST_CASE("meta with no primaries to amplify is harmless", "[meta]") {
    World world;
    EntityStats stats;
    stats.add_modifier(flat_fire_damage(100.0f, GearRing));
    stats.add_modifier(meta(tags::Spell, 0.50f, AuraSpell));

    EvalContext ctx;

    REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                 WithinAbs(100.0f, Tolerance));
}

TEST_CASE("meta with broader tag requirement does not match narrower primary", "[meta]") {
    World world;
    EntityStats stats;
    stats.add_modifier(increased_fire_damage(0.20f, GearAmulet, tags::Spell));
    stats.add_modifier(meta(tags::Spell | tags::Fire, 0.50f, AuraSpell));
    stats.add_modifier(flat_fire_damage(100.0f, GearRing, tags::Spell));

    EvalContext ctx{.ability_tags = tags::Spell | tags::Fire};

    REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                 WithinAbs(100.0f * 1.20f, Tolerance));
}

TEST_CASE("meta with narrower tag requirement matches broader primary", "[meta]") {
    World world;
    EntityStats stats;
    stats.add_modifier(increased_fire_damage(0.20f, GearAmulet, tags::Spell | tags::Fire));
    stats.add_modifier(meta(tags::Spell, 0.50f, AuraSpell));
    stats.add_modifier(flat_fire_damage(100.0f, GearRing, tags::Spell | tags::Fire));

    EvalContext ctx{.ability_tags = tags::Spell | tags::Fire};

    REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                 WithinAbs(100.0f * 1.50f * (1.0f + (0.20f * 1.50f)), Tolerance));
}

TEST_CASE("meta with condition only amplifies when condition is met", "[meta]") {
    World world;
    auto attacker = world.spawn(EntityKind::Player);
    world.health(attacker) = Health{.current = 100.0f, .max = 100.0f};

    EntityStats stats;
    stats.add_modifier(flat_fire_damage(100.0f, GearRing, tags::Spell));

    Modifier conditional_meta{
        .stat = StatId::None,
        .op = ModOp::Increased,
        .phase = ModPhase::Meta,
        .condition = ConditionId::WhileAtFullLife,
        .required_tags = tags::Spell,
        .magnitude = 0.50f,
        .source = AuraSpell,
    };
    stats.add_modifier(conditional_meta);

    EvalContext ctx{.attacker = attacker, .ability_tags = tags::Spell};

    SECTION("at full life: meta amplifies") {
        REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                     WithinAbs(150.0f, Tolerance));
    }

    SECTION("not at full life: meta does not amplify") {
        world.health(attacker).current = 50.0f;
        REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                     WithinAbs(100.0f, Tolerance));
    }
}

TEST_CASE("meta does not amplify primary that fails its own condition", "[meta]") {
    World world;
    auto attacker = world.spawn(EntityKind::Player);
    world.health(attacker) = Health{.current = 50.0f, .max = 100.0f};

    EntityStats stats;
    stats.add_modifier(flat_fire_damage(100.0f, GearRing));

    Modifier conditional_primary{
        .stat = StatId::FireDamage,
        .op = ModOp::Increased,
        .condition = ConditionId::WhileAtFullLife,
        .required_tags = tags::Spell,
        .magnitude = 0.20f,
        .source = GearAmulet,
    };
    stats.add_modifier(conditional_primary);
    stats.add_modifier(meta(tags::Spell, 0.50f, AuraSpell));

    EvalContext ctx{.attacker = attacker, .ability_tags = tags::Spell};

    REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                 WithinAbs(100.0f, Tolerance));
}