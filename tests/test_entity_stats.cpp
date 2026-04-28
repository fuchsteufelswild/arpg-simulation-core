#include "sim/entity_stats.hpp"
#include "sim/tags.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace sim;

namespace {

constexpr SourceId GearRing = make_source_id(source_categories::Gear, 1);
constexpr SourceId GearAmulet = make_source_id(source_categories::Gear, 2);
constexpr SourceId AuraAnger = make_source_id(source_categories::Aura, 1);

Modifier increased_fire_damage(SimFloat magnitude, SourceId source) {
    return Modifier{
        .stat = StatId::FireDamage,
        .op = ModOp::Increased,
        .magnitude = magnitude,
        .source = source,
    };
}

Modifier flat_max_life(SimFloat magnitude, SourceId source) {
    return Modifier{
        .stat = StatId::MaxLife,
        .op = ModOp::Flat,
        .magnitude = magnitude,
        .source = source,
    };
}

}  // namespace

TEST_CASE("default EntityStats has no buckets", "[entity_stats]") {
    EntityStats stats;
    REQUIRE(stats.bucket_count() == 0);
    REQUIRE_FALSE(stats.has_stat(StatId::FireDamage));
    REQUIRE(stats.primary_modifiers(StatId::FireDamage).empty());
    REQUIRE(stats.meta_modifiers().empty());
}

TEST_CASE("adding a modifier creates a bucket", "[entity_stats]") {
    EntityStats stats;
    stats.add_modifier(increased_fire_damage(0.20f, GearRing));

    REQUIRE(stats.bucket_count() == 1);
    REQUIRE(stats.has_stat(StatId::FireDamage));
    REQUIRE(stats.primary_modifiers(StatId::FireDamage).size() == 1);
    REQUIRE(stats.primary_modifiers(StatId::FireDamage)[0].magnitude == 0.20f);
}

TEST_CASE("multiple modifiers on the same stat share a bucket", "[entity_stats]") {
    EntityStats stats;
    stats.add_modifier(increased_fire_damage(0.20f, GearRing));
    stats.add_modifier(increased_fire_damage(0.30f, GearAmulet));

    REQUIRE(stats.bucket_count() == 1);
    REQUIRE(stats.primary_modifiers(StatId::FireDamage).size() == 2);
}

TEST_CASE("modifiers on different stats create distinct buckets", "[entity_stats]") {
    EntityStats stats;
    stats.add_modifier(increased_fire_damage(0.20f, GearRing));
    stats.add_modifier(flat_max_life(50.0f, GearAmulet));

    REQUIRE(stats.bucket_count() == 2);
    REQUIRE(stats.primary_modifiers(StatId::FireDamage).size() == 1);
    REQUIRE(stats.primary_modifiers(StatId::MaxLife).size() == 1);
}

TEST_CASE("meta modifiers go to a separate list", "[entity_stats]") {
    EntityStats stats;

    Modifier meta{
        .stat = StatId::None,
        .op = ModOp::Increased,
        .phase = ModPhase::Meta,
        .required_tags = tags::Spell,
        .magnitude = 0.50f,
        .source = AuraAnger,
    };
    stats.add_modifier(meta);

    REQUIRE(stats.bucket_count() == 0);
    REQUIRE(stats.meta_modifiers().size() == 1);
    REQUIRE(stats.meta_modifiers()[0].magnitude == 0.50f);
}

TEST_CASE("remove_modifiers_by_source removes only matching modifiers", "[entity_stats]") {
    EntityStats stats;
    stats.add_modifier(increased_fire_damage(0.20f, GearRing));
    stats.add_modifier(increased_fire_damage(0.30f, GearAmulet));
    stats.add_modifier(flat_max_life(50.0f, GearRing));

    const uint32_t removed = stats.remove_modifiers_by_source(GearRing);

    REQUIRE(removed == 2);
    REQUIRE(stats.primary_modifiers(StatId::FireDamage).size() == 1);
    REQUIRE(stats.primary_modifiers(StatId::FireDamage)[0].magnitude == 0.30f);
    REQUIRE(stats.primary_modifiers(StatId::MaxLife).empty());
}

TEST_CASE("remove_modifiers_by_source removes meta modifiers too", "[entity_stats]") {
    EntityStats stats;

    Modifier meta{
        .stat = StatId::None,
        .phase = ModPhase::Meta,
        .source = AuraAnger,
    };
    stats.add_modifier(meta);
    stats.add_modifier(increased_fire_damage(0.20f, AuraAnger));

    const uint32_t removed = stats.remove_modifiers_by_source(AuraAnger);

    REQUIRE(removed == 2);
    REQUIRE(stats.meta_modifiers().empty());
    REQUIRE(stats.primary_modifiers(StatId::FireDamage).empty());
}

TEST_CASE("remove with no matches returns zero", "[entity_stats]") {
    EntityStats stats;
    stats.add_modifier(increased_fire_damage(0.20f, GearRing));

    const uint32_t removed = stats.remove_modifiers_by_source(GearAmulet);

    REQUIRE(removed == 0);
    REQUIRE(stats.primary_modifiers(StatId::FireDamage).size() == 1);
}

TEST_CASE("buckets are kept sorted by StatId", "[entity_stats]") {
    EntityStats stats;
    stats.add_modifier(flat_max_life(50.0f, GearRing));
    stats.add_modifier(increased_fire_damage(0.20f, GearRing));

    REQUIRE(stats.bucket_count() == 2);
    REQUIRE(stats.has_stat(StatId::MaxLife));
    REQUIRE(stats.has_stat(StatId::FireDamage));
}

TEST_CASE("clear removes all modifiers", "[entity_stats]") {
    EntityStats stats;
    stats.add_modifier(increased_fire_damage(0.20f, GearRing));
    stats.add_modifier(flat_max_life(50.0f, GearAmulet));

    stats.clear();

    REQUIRE(stats.bucket_count() == 0);
    REQUIRE(stats.meta_modifiers().empty());
}

TEST_CASE("adding a modifier marks bucket dirty", "[entity_stats]") {
    EntityStats stats;
    stats.add_modifier(increased_fire_damage(0.20f, GearRing));

    auto mods = stats.primary_modifiers(StatId::FireDamage);
    REQUIRE(mods.size() == 1);
}