#include "sim/data/ability_loader.hpp"

#include "sim/ability_definition.hpp"
#include "sim/data/enum_lookup.hpp"
#include "sim/effect.hpp"

#include <toml++/toml.hpp>

#include <fstream>
#include <sstream>

namespace sim::data {

namespace {

[[nodiscard]] TagMask parse_tags_node(const toml::node* node) {
    if (node == nullptr) {
        return tags::None;
    }
    if (auto str = node->value<std::string>()) {
        return parse_tag_list(*str);
    }
    if (const auto* arr = node->as_array()) {
        TagMask result = tags::None;
        for (const auto& entry : *arr) {
            if (auto name = entry.value<std::string>()) {
                if (auto tag = tag_from_string(*name)) {
                    result |= *tag;
                }
            }
        }
        return result;
    }
    return tags::None;
}

[[nodiscard]] std::expected<Effect, LoadError> parse_effect(const toml::table& effect_table) {
    const auto type_str = effect_table["type"].value<std::string>();
    if (!type_str) {
        return std::unexpected(LoadError{"effect missing 'type' field"});
    }

    if (*type_str == "Damage") {
        const auto stat_str = effect_table["damage_stat"].value<std::string>();
        if (!stat_str) {
            return std::unexpected(LoadError{"Damage effect missing 'damage_stat'"});
        }
        const auto stat = stat_from_string(*stat_str);
        if (!stat) {
            return std::unexpected(LoadError{"unknown stat: " + *stat_str});
        }
        return DamageEffect{
            .damage_stat = *stat,
            .base_amount = effect_table["base_amount"].value_or(0.0f),
        };
    }

    if (*type_str == "ApplyStatus") {
        const auto status_str = effect_table["status"].value<std::string>();
        if (!status_str) {
            return std::unexpected(LoadError{"ApplyStatus effect missing 'status'"});
        }
        const auto status = status_from_string(*status_str);
        if (!status) {
            return std::unexpected(LoadError{"unknown status: " + *status_str});
        }
        return ApplyStatusEffect{
            .status = *status,
            .chance = effect_table["chance"].value_or(1.0f),
            .magnitude = effect_table["magnitude"].value_or(0.0f),
            .duration_ticks = effect_table["duration_ticks"].value_or(0u),
        };
    }

    if (*type_str == "Chain") {
        return ChainEffect{
            .max_chains = static_cast<uint8_t>(effect_table["max_chains"].value_or(0)),
            .falloff = effect_table["falloff"].value_or(1.0f),
            .radius = effect_table["radius"].value_or(0.0f),
        };
    }

    if (*type_str == "Trigger") {
        return TriggerEffect{
            .ability_to_trigger =
                static_cast<AbilityId>(effect_table["ability_to_trigger"].value_or(0)),
            .chance = effect_table["chance"].value_or(1.0f),
        };
    }

    return std::unexpected(LoadError{"unknown effect type: " + *type_str});
}

[[nodiscard]] std::expected<AbilityDefinition, LoadError>
parse_ability(const toml::table& ability_table) {
    AbilityDefinition def;

    if (auto name = ability_table["name"].value<std::string>()) {
        def.name = *name;
    }

    def.tags = parse_tags_node(ability_table["tags"].node());
    def.cast_time_ticks = ability_table["cast_time_ticks"].value_or(0u);
    def.travel_speed = ability_table["travel_speed"].value_or(0.0f);

    if (const auto* effects_array = ability_table["effects"].as_array()) {
        for (const auto& effect_node : *effects_array) {
            const auto* const effect_table = effect_node.as_table();
            if (effect_table == nullptr) {
                return std::unexpected(LoadError{"effect entry is not a table"});
            }

            auto parsed = parse_effect(*effect_table);
            if (!parsed) {
                return std::unexpected(parsed.error());
            }
            def.effects.push_back(std::move(*parsed));
        }
    }

    return def;
}

}  // namespace

std::expected<uint32_t, LoadError> load_abilities_from_string(std::string_view toml_text,
                                                              AbilityRegistry& registry) {
    toml::table root;
    try {
        root = toml::parse(toml_text);
    } catch (const toml::parse_error& e) {
        std::stringstream ss;
        ss << "TOML parse error: " << e.description();
        return std::unexpected(LoadError{ss.str()});
    }

    const auto abilities_array = root["ability"].as_array();
    if (abilities_array == nullptr) {
        return 0u;
    }

    uint32_t count = 0;
    for (const auto& ability_node : *abilities_array) {
        const auto ability_table = ability_node.as_table();
        if (ability_table == nullptr) {
            return std::unexpected(LoadError{"ability entry is not a table"});
        }

        auto parsed = parse_ability(*ability_table);
        if (!parsed) {
            return std::unexpected(parsed.error());
        }

        registry.register_ability(std::move(*parsed));
        ++count;
    }

    return count;
}

std::expected<uint32_t, LoadError> load_abilities_from_file(std::string_view path,
                                                            AbilityRegistry& registry) {
    std::ifstream file{std::string{path}};
    if (!file.is_open()) {
        return std::unexpected(LoadError{"could not open file: " + std::string{path}});
    }

    std::stringstream ss;
    ss << file.rdbuf();
    return load_abilities_from_string(ss.str(), registry);
}

}  // namespace sim::data