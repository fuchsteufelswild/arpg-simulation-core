#include "sim/data/archetype_loader.hpp"

#include "sim/data/enum_lookup.hpp"
#include "sim/entity_archetype.hpp"

#include <toml++/toml.hpp>

#include <fstream>
#include <sstream>

namespace sim::data {

namespace {

[[nodiscard]] std::expected<AIArchetype, LoadError> parse_ai(const toml::table& ai_table,
                                                             const AbilityRegistry& abilities) {
    const auto ability_name = ai_table["preferred_ability"].value<std::string>();
    if (!ability_name) {
        return std::unexpected(LoadError{"ai block missing 'preferred_ability'"});
    }

    const AbilityId ability_id = abilities.find_by_name(*ability_name);
    if (ability_id == NoAbility) {
        return std::unexpected(LoadError{"unknown ability: " + *ability_name});
    }

    return AIArchetype{
        .preferred_ability = ability_id,
        .attack_range = ai_table["attack_range"].value_or(0.0f),
        .detection_range = ai_table["detection_range"].value_or(0.0f),
        .leash_range = ai_table["leash_range"].value_or(0.0f),
        .move_speed = ai_table["move_speed"].value_or(0.0f),
    };
}

[[nodiscard]] std::expected<EntityArchetype, LoadError>
parse_archetype(const toml::table& archetype_table, const AbilityRegistry& abilities) {
    EntityArchetype archetype;

    const auto name = archetype_table["name"].value<std::string>();
    if (!name) {
        return std::unexpected(LoadError{"archetype missing 'name'"});
    }
    archetype.name = *name;

    const auto kind_str = archetype_table["kind"].value<std::string>();
    if (!kind_str) {
        return std::unexpected(LoadError{"archetype '" + archetype.name + "' missing 'kind'"});
    }
    const auto kind = kind_from_string(*kind_str);
    if (!kind) {
        return std::unexpected(LoadError{"unknown entity kind: " + *kind_str});
    }
    archetype.kind = *kind;

    archetype.max_health = archetype_table["max_health"].value_or(0.0f);

    if (const auto* ai_table = archetype_table["ai"].as_table()) {
        auto parsed = parse_ai(*ai_table, abilities);
        if (!parsed) {
            return std::unexpected(parsed.error());
        }
        archetype.ai = *parsed;
    }

    return archetype;
}

}  // namespace

std::expected<uint32_t, LoadError> load_archetypes_from_string(std::string_view toml_text,
                                                               const AbilityRegistry& abilities,
                                                               EntityArchetypeRegistry& registry) {
    toml::table root;
    try {
        root = toml::parse(toml_text);
    } catch (const toml::parse_error& e) {
        std::stringstream ss;
        ss << "TOML parse error: " << e.description();
        return std::unexpected(LoadError{ss.str()});
    }

    const auto* archetypes_array = root["archetype"].as_array();
    if (archetypes_array == nullptr) {
        return 0u;
    }

    uint32_t count = 0;
    for (const auto& archetype_node : *archetypes_array) {
        const auto* const archetype_table = archetype_node.as_table();
        if (archetype_table == nullptr) {
            return std::unexpected(LoadError{"archetype entry is not a table"});
        }

        auto parsed = parse_archetype(*archetype_table, abilities);
        if (!parsed) {
            return std::unexpected(parsed.error());
        }

        registry.register_archetype(std::move(*parsed));
        ++count;
    }

    return count;
}

std::expected<uint32_t, LoadError> load_archetypes_from_file(std::string_view path,
                                                             const AbilityRegistry& abilities,
                                                             EntityArchetypeRegistry& registry) {
    std::ifstream file{std::string{path}};
    if (!file.is_open()) {
        return std::unexpected(LoadError{"could not open file: " + std::string{path}});
    }

    std::stringstream ss;
    ss << file.rdbuf();
    return load_archetypes_from_string(ss.str(), abilities, registry);
}

}  // namespace sim::data