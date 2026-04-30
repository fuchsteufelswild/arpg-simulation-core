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
constexpr SourceId GearChest = make_source_id(source_categories::Gear, 3);

Modifier flat(StatId stat, SimFloat amount, SourceId source, TagMask required = tags::None) {
    return Modifier{
        .stat = stat,
        .op = ModOp::Flat,
        .required_tags = required,
        .magnitude = amount,
        .source = source,
    };
}

Modifier increased(StatId stat, SimFloat fraction, SourceId source, TagMask required = tags::None) {
    return Modifier{
        .stat = stat,
        .op = ModOp::Increased,
        .required_tags = required,
        .magnitude = fraction,
        .source = source,
    };
}

Modifier more(StatId stat, SimFloat fraction, SourceId source, TagMask required = tags::None) {
    return Modifier{
        .stat = stat,
        .op = ModOp::More,
        .required_tags = required,
        .magnitude = fraction,
        .source = source,
    };
}

}  // namespace

TEST_CASE("evaluate with no modifiers returns zero", "[evaluate]") {
    World world;

    EntityStats stats;
    EvalContext ctx;

    REQUIRE(evaluate_stat(world, stats, StatId::FireDamage, ctx) == 0.0f);
}

TEST_CASE("single flat modifier returns the flat value", "[evaluate]") {
    World world;

    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 50.0f, GearRing));
    EvalContext ctx;

    REQUIRE(evaluate_stat(world, stats, StatId::FireDamage, ctx) == 50.0f);
}

TEST_CASE("multiple flat modifiers sum", "[evaluate]") {
    World world;

    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 50.0f, GearRing));
    stats.add_modifier(flat(StatId::FireDamage, 30.0f, GearAmulet));
    EvalContext ctx;

    REQUIRE(evaluate_stat(world, stats, StatId::FireDamage, ctx) == 80.0f);
}

TEST_CASE("increased modifiers add together additively", "[evaluate]") {
    World world;

    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));
    stats.add_modifier(increased(StatId::FireDamage, 0.20f, GearAmulet));
    stats.add_modifier(increased(StatId::FireDamage, 0.30f, GearChest));
    EvalContext ctx;

    REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                 WithinAbs(150.0f, Tolerance));
}

TEST_CASE("more modifiers multiply each other", "[evaluate]") {
    World world;

    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));
    stats.add_modifier(more(StatId::FireDamage, 0.20f, GearAmulet));
    stats.add_modifier(more(StatId::FireDamage, 0.20f, GearChest));
    EvalContext ctx;

    REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                 WithinAbs(144.0f, Tolerance));
}

TEST_CASE("full formula: flat then increased then more", "[evaluate]") {
    World world;

    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));
    stats.add_modifier(increased(StatId::FireDamage, 0.50f, GearAmulet));
    stats.add_modifier(more(StatId::FireDamage, 0.20f, GearChest));
    EvalContext ctx;

    REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                 WithinAbs(180.0f, Tolerance));
}

TEST_CASE("modifier with required tag does not apply when context lacks it", "[evaluate]") {
    World world;

    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));
    stats.add_modifier(increased(StatId::FireDamage, 0.50f, GearAmulet, tags::Spell));
    EvalContext ctx{.ability_tags = tags::Attack};

    REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                 WithinAbs(100.0f, Tolerance));
}

TEST_CASE("modifier with required tag applies when context has it", "[evaluate]") {
    World world;

    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));
    stats.add_modifier(increased(StatId::FireDamage, 0.50f, GearAmulet, tags::Spell));
    EvalContext ctx{.ability_tags = tags::Spell | tags::Fire};

    REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                 WithinAbs(150.0f, Tolerance));
}

TEST_CASE("modifier requiring multiple tags only applies if all match", "[evaluate]") {
    World world;

    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));
    stats.add_modifier(increased(StatId::FireDamage, 0.50f, GearAmulet, tags::Spell | tags::Fire));

    SECTION("only one tag present") {
        EvalContext ctx{.ability_tags = tags::Spell};
        REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                     WithinAbs(100.0f, Tolerance));
    }

    SECTION("both tags present") {
        EvalContext ctx{.ability_tags = tags::Spell | tags::Fire};
        REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                     WithinAbs(150.0f, Tolerance));
    }

    SECTION("both tags plus extra") {
        EvalContext ctx{.ability_tags = tags::Spell | tags::Fire | tags::Projectile};
        REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                     WithinAbs(150.0f, Tolerance));
    }
}

TEST_CASE("queries on different stats are independent", "[evaluate]") {
    World world;

    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));
    stats.add_modifier(flat(StatId::ColdDamage, 50.0f, GearAmulet));
    EvalContext ctx;

    REQUIRE(evaluate_stat(world, stats, StatId::FireDamage, ctx) == 100.0f);
    REQUIRE(evaluate_stat(world, stats, StatId::ColdDamage, ctx) == 50.0f);
    REQUIRE(evaluate_stat(world, stats, StatId::LightningDamage, ctx) == 0.0f);
}

TEST_CASE("realistic ARPG scenario: fireball with modifier stack", "[evaluate]") {
    World world;

    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 40.0f, GearRing));
    stats.add_modifier(increased(StatId::FireDamage, 0.30f, GearAmulet));
    stats.add_modifier(increased(StatId::FireDamage, 0.40f, GearChest, tags::Spell));
    stats.add_modifier(
        increased(StatId::FireDamage, 0.20f, make_source_id(source_categories::PassiveTree, 1)));
    stats.add_modifier(more(
        StatId::FireDamage, 0.30f, make_source_id(source_categories::PassiveTree, 2), tags::Spell));

    SECTION("when casting a fire spell") {
        EvalContext ctx{.ability_tags = tags::Spell | tags::Fire | tags::Projectile};
        const SimFloat dmg = evaluate_stat(world, stats, StatId::FireDamage, ctx);
        REQUIRE_THAT(dmg, WithinAbs(40.0f * 1.90f * 1.30f, Tolerance));
    }

    SECTION("when using a fire attack") {
        EvalContext ctx{.ability_tags = tags::Attack | tags::Fire};
        const SimFloat dmg = evaluate_stat(world, stats, StatId::FireDamage, ctx);
        REQUIRE_THAT(dmg, WithinAbs(40.0f * 1.50f, Tolerance));
    }
}

TEST_CASE("conditional modifier applies when condition is met", "[evaluate][conditions]") {
    World world;
    auto attacker = world.spawn(EntityKind::Player);
    world.health(attacker) = Health{.current = 100.0f, .max = 100.0f};

    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));

    Modifier conditional{
        .stat = StatId::FireDamage,
        .op = ModOp::Increased,
        .condition = ConditionId::WhileAtFullLife,
        .magnitude = 0.30f,
        .source = GearAmulet,
    };
    stats.add_modifier(conditional);

    EvalContext ctx{.attacker = attacker};

    REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                 WithinAbs(130.0f, Tolerance));
}

TEST_CASE("conditional modifier does not apply when condition fails", "[evaluate][conditions]") {
    World world;
    auto attacker = world.spawn(EntityKind::Player);
    world.health(attacker) = Health{.current = 50.0f, .max = 100.0f};

    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));

    Modifier conditional{
        .stat = StatId::FireDamage,
        .op = ModOp::Increased,
        .condition = ConditionId::WhileAtFullLife,
        .magnitude = 0.30f,
        .source = GearAmulet,
    };
    stats.add_modifier(conditional);

    EvalContext ctx{.attacker = attacker};

    REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                 WithinAbs(100.0f, Tolerance));
}

TEST_CASE("against-chilled modifier requires target with chilled tag", "[evaluate][conditions]") {
    World world;
    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));

    Modifier conditional{
        .stat = StatId::FireDamage,
        .op = ModOp::Increased,
        .condition = ConditionId::AgainstChilled,
        .magnitude = 0.40f,
        .source = GearAmulet,
    };
    stats.add_modifier(conditional);

    SECTION("target is chilled") {
        EvalContext ctx{.target_tags = tags::Chilled};
        REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                     WithinAbs(140.0f, Tolerance));
    }

    SECTION("target is not chilled") {
        EvalContext ctx{.target_tags = tags::None};
        REQUIRE_THAT(evaluate_stat(world, stats, StatId::FireDamage, ctx),
                     WithinAbs(100.0f, Tolerance));
    }
}