#include "sim/entity_stats.hpp"
#include "sim/eval_context.hpp"
#include "sim/evaluate.hpp"

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
    EntityStats stats;
    EvalContext ctx;

    REQUIRE(evaluate_stat(stats, StatId::FireDamage, ctx) == 0.0f);
}

TEST_CASE("single flat modifier returns the flat value", "[evaluate]") {
    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 50.0f, GearRing));
    EvalContext ctx;

    REQUIRE(evaluate_stat(stats, StatId::FireDamage, ctx) == 50.0f);
}

TEST_CASE("multiple flat modifiers sum", "[evaluate]") {
    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 50.0f, GearRing));
    stats.add_modifier(flat(StatId::FireDamage, 30.0f, GearAmulet));
    EvalContext ctx;

    REQUIRE(evaluate_stat(stats, StatId::FireDamage, ctx) == 80.0f);
}

TEST_CASE("increased modifiers add together additively", "[evaluate]") {
    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));
    stats.add_modifier(increased(StatId::FireDamage, 0.20f, GearAmulet));
    stats.add_modifier(increased(StatId::FireDamage, 0.30f, GearChest));
    EvalContext ctx;

    REQUIRE_THAT(evaluate_stat(stats, StatId::FireDamage, ctx), WithinAbs(150.0f, Tolerance));
}

TEST_CASE("more modifiers multiply each other", "[evaluate]") {
    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));
    stats.add_modifier(more(StatId::FireDamage, 0.20f, GearAmulet));
    stats.add_modifier(more(StatId::FireDamage, 0.20f, GearChest));
    EvalContext ctx;

    REQUIRE_THAT(evaluate_stat(stats, StatId::FireDamage, ctx), WithinAbs(144.0f, Tolerance));
}

TEST_CASE("full formula: flat then increased then more", "[evaluate]") {
    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));
    stats.add_modifier(increased(StatId::FireDamage, 0.50f, GearAmulet));
    stats.add_modifier(more(StatId::FireDamage, 0.20f, GearChest));
    EvalContext ctx;

    REQUIRE_THAT(evaluate_stat(stats, StatId::FireDamage, ctx), WithinAbs(180.0f, Tolerance));
}

TEST_CASE("modifier with required tag does not apply when context lacks it", "[evaluate]") {
    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));
    stats.add_modifier(increased(StatId::FireDamage, 0.50f, GearAmulet, tags::Spell));
    EvalContext ctx{.ability_tags = tags::Attack};

    REQUIRE_THAT(evaluate_stat(stats, StatId::FireDamage, ctx), WithinAbs(100.0f, Tolerance));
}

TEST_CASE("modifier with required tag applies when context has it", "[evaluate]") {
    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));
    stats.add_modifier(increased(StatId::FireDamage, 0.50f, GearAmulet, tags::Spell));
    EvalContext ctx{.ability_tags = tags::Spell | tags::Fire};

    REQUIRE_THAT(evaluate_stat(stats, StatId::FireDamage, ctx), WithinAbs(150.0f, Tolerance));
}

TEST_CASE("modifier requiring multiple tags only applies if all match", "[evaluate]") {
    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));
    stats.add_modifier(increased(StatId::FireDamage, 0.50f, GearAmulet, tags::Spell | tags::Fire));

    SECTION("only one tag present") {
        EvalContext ctx{.ability_tags = tags::Spell};
        REQUIRE_THAT(evaluate_stat(stats, StatId::FireDamage, ctx), WithinAbs(100.0f, Tolerance));
    }

    SECTION("both tags present") {
        EvalContext ctx{.ability_tags = tags::Spell | tags::Fire};
        REQUIRE_THAT(evaluate_stat(stats, StatId::FireDamage, ctx), WithinAbs(150.0f, Tolerance));
    }

    SECTION("both tags plus extra") {
        EvalContext ctx{.ability_tags = tags::Spell | tags::Fire | tags::Projectile};
        REQUIRE_THAT(evaluate_stat(stats, StatId::FireDamage, ctx), WithinAbs(150.0f, Tolerance));
    }
}

TEST_CASE("queries on different stats are independent", "[evaluate]") {
    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));
    stats.add_modifier(flat(StatId::ColdDamage, 50.0f, GearAmulet));
    EvalContext ctx;

    REQUIRE(evaluate_stat(stats, StatId::FireDamage, ctx) == 100.0f);
    REQUIRE(evaluate_stat(stats, StatId::ColdDamage, ctx) == 50.0f);
    REQUIRE(evaluate_stat(stats, StatId::LightningDamage, ctx) == 0.0f);
}

TEST_CASE("meta modifiers do not affect primary stat queries", "[evaluate]") {
    EntityStats stats;
    stats.add_modifier(flat(StatId::FireDamage, 100.0f, GearRing));

    Modifier meta{
        .stat = StatId::None,
        .op = ModOp::More,
        .phase = ModPhase::Meta,
        .magnitude = 0.50f,
        .source = GearAmulet,
    };
    stats.add_modifier(meta);

    EvalContext ctx;

    REQUIRE_THAT(evaluate_stat(stats, StatId::FireDamage, ctx), WithinAbs(100.0f, Tolerance));
}

TEST_CASE("realistic ARPG scenario: fireball with modifier stack", "[evaluate]") {
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
        const SimFloat dmg = evaluate_stat(stats, StatId::FireDamage, ctx);
        REQUIRE_THAT(dmg, WithinAbs(40.0f * 1.90f * 1.30f, Tolerance));
    }

    SECTION("when using a fire attack") {
        EvalContext ctx{.ability_tags = tags::Attack | tags::Fire};
        const SimFloat dmg = evaluate_stat(stats, StatId::FireDamage, ctx);
        REQUIRE_THAT(dmg, WithinAbs(40.0f * 1.50f, Tolerance));
    }
}