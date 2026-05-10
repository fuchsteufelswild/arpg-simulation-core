#include "sim/data/ability_loader.hpp"
#include "sim/data/enum_lookup.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace sim;
using namespace sim::data;

TEST_CASE("stat names map to enum values", "[data][lookup]") {
    REQUIRE(stat_from_string("FireDamage") == StatId::FireDamage);
    REQUIRE(stat_from_string("MaxLife") == StatId::MaxLife);
    REQUIRE(stat_from_string("CritChance") == StatId::CritChance);
}

TEST_CASE("unknown stat name returns nullopt", "[data][lookup]") {
    REQUIRE(stat_from_string("FireDmg") == std::nullopt);
    REQUIRE(stat_from_string("") == std::nullopt);
}

TEST_CASE("tag list parses pipe-separated names", "[data][lookup]") {
    const TagMask result = parse_tag_list("Fire | Spell | Projectile");
    REQUIRE(result == (tags::Fire | tags::Spell | tags::Projectile));
}

TEST_CASE("tag list ignores unknown tags", "[data][lookup]") {
    const TagMask result = parse_tag_list("Fire | Garbage | Spell");
    REQUIRE(result == (tags::Fire | tags::Spell));
}

TEST_CASE("status names map to enum values", "[data][lookup]") {
    REQUIRE(status_from_string("Chill") == StatusType::Chill);
    REQUIRE(status_from_string("Ignite") == StatusType::Ignite);
    REQUIRE(status_from_string("Unknown") == std::nullopt);
}

TEST_CASE("loads single ability with damage effect", "[data][toml]") {
    AbilityRegistry registry;

    constexpr std::string_view toml_text = R"(
[[ability]]
name = "Fireball"
tags = ["Fire", "Spell"]
cast_time_ticks = 15

[[ability.effects]]
type = "Damage"
damage_stat = "FireDamage"
base_amount = 50.0
)";

    auto result = load_abilities_from_string(toml_text, registry);
    REQUIRE(result.has_value());
    REQUIRE(*result == 1);
    REQUIRE(registry.count() == 1);
}

TEST_CASE("loaded ability has correct fields", "[data][toml]") {
    AbilityRegistry registry;

    constexpr std::string_view toml_text = R"(
[[ability]]
name = "Fireball"
tags = ["Fire", "Spell"]
cast_time_ticks = 15
travel_speed = 25.0

[[ability.effects]]
type = "Damage"
damage_stat = "FireDamage"
base_amount = 50.0
)";

    auto result = load_abilities_from_string(toml_text, registry);
    REQUIRE(result.has_value());

    const auto* def = registry.find(1);
    REQUIRE(def != nullptr);
    REQUIRE(def->name == "Fireball");
    REQUIRE((def->tags & tags::Fire) != 0);
    REQUIRE((def->tags & tags::Spell) != 0);
    REQUIRE(def->cast_time_ticks == 15);
    REQUIRE(def->travel_speed == 25.0f);
    REQUIRE(def->effects.size() == 1);

    const auto* dmg = std::get_if<DamageEffect>(def->effects.data());
    REQUIRE(dmg != nullptr);
    REQUIRE(dmg->damage_stat == StatId::FireDamage);
    REQUIRE(dmg->base_amount == 50.0f);
}

TEST_CASE("loads multiple abilities", "[data][toml]") {
    AbilityRegistry registry;

    constexpr std::string_view toml_text = R"(
[[ability]]
name = "Fireball"
[[ability.effects]]
type = "Damage"
damage_stat = "FireDamage"
base_amount = 50.0

[[ability]]
name = "IceNova"
[[ability.effects]]
type = "Damage"
damage_stat = "ColdDamage"
base_amount = 30.0
)";

    auto result = load_abilities_from_string(toml_text, registry);
    REQUIRE(result.has_value());
    REQUIRE(*result == 2);
    REQUIRE(registry.count() == 2);
}

TEST_CASE("loads composite ability with chain and status", "[data][toml]") {
    AbilityRegistry registry;

    constexpr std::string_view toml_text = R"(
[[ability]]
name = "Fireball"
tags = ["Fire", "Spell", "Projectile"]

[[ability.effects]]
type = "Damage"
damage_stat = "FireDamage"
base_amount = 40.0

[[ability.effects]]
type = "ApplyStatus"
status = "Ignite"
chance = 0.25
magnitude = 5.0
duration_ticks = 120

[[ability.effects]]
type = "Chain"
max_chains = 3
falloff = 0.8
radius = 10.0
)";

    auto result = load_abilities_from_string(toml_text, registry);
    REQUIRE(result.has_value());

    const auto* def = registry.find(1);
    REQUIRE(def != nullptr);
    REQUIRE(def->effects.size() == 3);
    REQUIRE(std::holds_alternative<DamageEffect>(def->effects[0]));
    REQUIRE(std::holds_alternative<ApplyStatusEffect>(def->effects[1]));
    REQUIRE(std::holds_alternative<ChainEffect>(def->effects[2]));
}

TEST_CASE("unknown stat name produces error", "[data][toml]") {
    AbilityRegistry registry;

    constexpr std::string_view toml_text = R"(
[[ability]]
name = "Fireball"
[[ability.effects]]
type = "Damage"
damage_stat = "FireDmg"
base_amount = 50.0
)";

    auto result = load_abilities_from_string(toml_text, registry);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().message.contains("FireDmg") != false);
}

TEST_CASE("malformed TOML produces error", "[data][toml]") {
    AbilityRegistry registry;

    constexpr std::string_view toml_text = "this is not valid TOML [[[";

    auto result = load_abilities_from_string(toml_text, registry);
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("empty TOML produces zero loaded", "[data][toml]") {
    AbilityRegistry registry;

    auto result = load_abilities_from_string("", registry);
    REQUIRE(result.has_value());
    REQUIRE(*result == 0);
}

TEST_CASE("missing ability_to_trigger defaults but is loadable", "[data][toml]") {
    AbilityRegistry registry;

    constexpr std::string_view toml_text = R"(
[[ability]]
name = "Fireball"
[[ability.effects]]
type = "Trigger"
)";

    auto result = load_abilities_from_string(toml_text, registry);
    REQUIRE(result.has_value());
}