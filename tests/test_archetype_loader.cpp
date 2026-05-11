#include "sim/data/ability_loader.hpp"
#include "sim/data/archetype_loader.hpp"
#include "sim/entity_archetype_registry.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace sim;
using namespace sim::data;

namespace {

constexpr std::string_view FireballToml = R"(
[[ability]]
name = "Fireball"
[[ability.effects]]
type = "Damage"
damage_stat = "FireDamage"
base_amount = 40.0
)";

[[nodiscard]] AbilityRegistry make_ability_registry() {
    AbilityRegistry registry;
    auto loaded = load_abilities_from_string(FireballToml, registry);
    REQUIRE(loaded.has_value());
    return registry;
}

}  // namespace

TEST_CASE("loads single enemy archetype with AI block", "[archetype][toml]") {
    const auto abilities = make_ability_registry();
    EntityArchetypeRegistry registry;

    constexpr std::string_view toml_text = R"(
[[archetype]]
name = "Goblin"
kind = "Enemy"
max_health = 80.0

[archetype.ai]
preferred_ability = "Fireball"
attack_range = 10.0
detection_range = 20.0
move_speed = 0.4
)";

    auto result = load_archetypes_from_string(toml_text, abilities, registry);
    REQUIRE(result.has_value());
    REQUIRE(*result == 1);
    REQUIRE(registry.size() == 1);
}

TEST_CASE("loaded enemy archetype has correct fields", "[archetype][toml]") {
    const auto abilities = make_ability_registry();
    EntityArchetypeRegistry registry;

    constexpr std::string_view toml_text = R"(
[[archetype]]
name = "Goblin"
kind = "Enemy"
max_health = 80.0

[archetype.ai]
preferred_ability = "Fireball"
attack_range = 10.0
detection_range = 20.0
leash_range = 30.0
move_speed = 0.4
)";

    auto result = load_archetypes_from_string(toml_text, abilities, registry);
    REQUIRE(result.has_value());

    const auto id = registry.find_by_name("Goblin");
    REQUIRE(id != NoArchetype);

    const EntityArchetype& def = registry.get(id);
    REQUIRE(def.name == "Goblin");
    REQUIRE(def.kind == EntityKind::Enemy);
    REQUIRE(def.max_health == 80.0f);
    REQUIRE(def.ai.has_value());
    REQUIRE(def.ai->preferred_ability != NoAbility);
    REQUIRE(def.ai->attack_range == 10.0f);
    REQUIRE(def.ai->detection_range == 20.0f);
    REQUIRE(def.ai->leash_range == 30.0f);
    REQUIRE(def.ai->move_speed == 0.4f);
}

TEST_CASE("loads player archetype without AI block", "[archetype][toml]") {
    const auto abilities = make_ability_registry();
    EntityArchetypeRegistry registry;

    constexpr std::string_view toml_text = R"(
[[archetype]]
name = "Player"
kind = "Player"
max_health = 500.0
)";

    auto result = load_archetypes_from_string(toml_text, abilities, registry);
    REQUIRE(result.has_value());

    const auto id = registry.find_by_name("Player");
    REQUIRE(id != NoArchetype);

    const EntityArchetype& def = registry.get(id);
    REQUIRE(def.kind == EntityKind::Player);
    REQUIRE(def.max_health == 500.0f);
    REQUIRE_FALSE(def.ai.has_value());
}

TEST_CASE("loads multiple archetypes", "[archetype][toml]") {
    const auto abilities = make_ability_registry();
    EntityArchetypeRegistry registry;

    constexpr std::string_view toml_text = R"(
[[archetype]]
name = "Goblin"
kind = "Enemy"
max_health = 80.0
[archetype.ai]
preferred_ability = "Fireball"
attack_range = 10.0
detection_range = 20.0

[[archetype]]
name = "Player"
kind = "Player"
max_health = 500.0
)";

    auto result = load_archetypes_from_string(toml_text, abilities, registry);
    REQUIRE(result.has_value());
    REQUIRE(*result == 2);
    REQUIRE(registry.size() == 2);
    REQUIRE(registry.find_by_name("Goblin") != NoArchetype);
    REQUIRE(registry.find_by_name("Player") != NoArchetype);
}

TEST_CASE("unknown entity kind produces error", "[archetype][toml]") {
    const auto abilities = make_ability_registry();
    EntityArchetypeRegistry registry;

    constexpr std::string_view toml_text = R"(
[[archetype]]
name = "Ghost"
kind = "Specter"
max_health = 50.0
)";

    auto result = load_archetypes_from_string(toml_text, abilities, registry);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().message.contains("Specter") != false);
}

TEST_CASE("unknown ability reference produces error", "[archetype][toml]") {
    const auto abilities = make_ability_registry();
    EntityArchetypeRegistry registry;

    constexpr std::string_view toml_text = R"(
[[archetype]]
name = "BrokenGoblin"
kind = "Enemy"
max_health = 80.0
[archetype.ai]
preferred_ability = "DoesNotExist"
attack_range = 10.0
detection_range = 20.0
)";

    auto result = load_archetypes_from_string(toml_text, abilities, registry);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().message.contains("DoesNotExist") != false);
}

TEST_CASE("missing name produces error", "[archetype][toml]") {
    const auto abilities = make_ability_registry();
    EntityArchetypeRegistry registry;

    constexpr std::string_view toml_text = R"(
[[archetype]]
kind = "Enemy"
max_health = 80.0
)";

    auto result = load_archetypes_from_string(toml_text, abilities, registry);
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("missing kind produces error", "[archetype][toml]") {
    const auto abilities = make_ability_registry();
    EntityArchetypeRegistry registry;

    constexpr std::string_view toml_text = R"(
[[archetype]]
name = "Goblin"
max_health = 80.0
)";

    auto result = load_archetypes_from_string(toml_text, abilities, registry);
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("ai block missing preferred_ability produces error", "[archetype][toml]") {
    const auto abilities = make_ability_registry();
    EntityArchetypeRegistry registry;

    constexpr std::string_view toml_text = R"(
[[archetype]]
name = "Goblin"
kind = "Enemy"
max_health = 80.0
[archetype.ai]
attack_range = 10.0
detection_range = 20.0
)";

    auto result = load_archetypes_from_string(toml_text, abilities, registry);
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("malformed TOML produces error", "[archetype][toml]") {
    const auto abilities = make_ability_registry();
    EntityArchetypeRegistry registry;

    constexpr std::string_view toml_text = "this is not valid TOML [[[";

    auto result = load_archetypes_from_string(toml_text, abilities, registry);
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("empty TOML produces zero loaded", "[archetype][toml]") {
    const auto abilities = make_ability_registry();
    EntityArchetypeRegistry registry;

    auto result = load_archetypes_from_string("", abilities, registry);
    REQUIRE(result.has_value());
    REQUIRE(*result == 0);
}

TEST_CASE("ai block uses default leash and move_speed when omitted", "[archetype][toml]") {
    const auto abilities = make_ability_registry();
    EntityArchetypeRegistry registry;

    constexpr std::string_view toml_text = R"(
[[archetype]]
name = "Goblin"
kind = "Enemy"
max_health = 80.0
[archetype.ai]
preferred_ability = "Fireball"
attack_range = 10.0
detection_range = 20.0
)";

    auto result = load_archetypes_from_string(toml_text, abilities, registry);
    REQUIRE(result.has_value());

    const EntityArchetype& def = registry.get(registry.find_by_name("Goblin"));
    REQUIRE(def.ai->leash_range == 0.0f);
    REQUIRE(def.ai->move_speed == 0.0f);
}