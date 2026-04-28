#include "sim/modifier.hpp"
#include "sim/tags.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace sim;

TEST_CASE("default modifier is well-defined", "[modifier]") {
    Modifier m;

    REQUIRE(m.stat == StatId::None);
    REQUIRE(m.op == ModOp::Flat);
    REQUIRE(m.phase == ModPhase::Primary);
    REQUIRE(m.condition == ConditionId::None);
    REQUIRE(m.required_tags == tags::None);
    REQUIRE(m.magnitude == 0.0f);
    REQUIRE(m.source == 0);
}

TEST_CASE("designated initialization works as expected", "[modifier]") {
    const Modifier m{
        .stat = StatId::FireDamage,
        .op = ModOp::Increased,
        .required_tags = tags::Fire | tags::Spell,
        .magnitude = 0.20f,
    };

    REQUIRE(m.stat == StatId::FireDamage);
    REQUIRE(m.op == ModOp::Increased);
    REQUIRE(m.magnitude == 0.20f);
    REQUIRE(m.required_tags == (tags::Fire | tags::Spell));
    REQUIRE(m.phase == ModPhase::Primary);
    REQUIRE(m.condition == ConditionId::None);
}

TEST_CASE("tags_match requires all bits to be present", "[tags]") {
    constexpr TagMask required = tags::Fire | tags::Spell;

    REQUIRE(tags_match(required, tags::Fire | tags::Spell));
    REQUIRE(tags_match(required, tags::Fire | tags::Spell | tags::Projectile));

    REQUIRE_FALSE(tags_match(required, tags::Fire));
    REQUIRE_FALSE(tags_match(required, tags::Spell));
    REQUIRE_FALSE(tags_match(required, tags::Cold | tags::Spell));
    REQUIRE_FALSE(tags_match(required, tags::None));
}

TEST_CASE("tags_match with no requirement always matches", "[tags]") {
    REQUIRE(tags_match(tags::None, tags::None));
    REQUIRE(tags_match(tags::None, tags::Fire));
    REQUIRE(tags_match(tags::None, tags::Fire | tags::Spell | tags::Projectile));
}

TEST_CASE("source IDs pack and unpack correctly", "[modifier]") {
    constexpr SourceId s = make_source_id(source_categories::Gear, 42);

    REQUIRE((s >> 16) == source_categories::Gear);
    REQUIRE((s & 0xFFFFu) == 42);
}

TEST_CASE("source IDs in different categories don't collide", "[modifier]") {
    constexpr SourceId gear = make_source_id(source_categories::Gear, 1);
    constexpr SourceId aura = make_source_id(source_categories::Aura, 1);

    REQUIRE(gear != aura);
}

TEST_CASE("StatCount reflects the enum", "[stat]") {
    REQUIRE(StatCount > 0);
    REQUIRE(StatCount < 256);
    REQUIRE(to_index(StatId::None) == 0);
    REQUIRE(to_index(StatId::FireDamage) > 0);
    REQUIRE(to_index(StatId::FireDamage) < StatCount);
}