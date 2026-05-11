#pragma once

#include "sim/ability_registry.hpp"
#include "sim/data/ability_loader.hpp"
#include "sim/entity_archetype_registry.hpp"

#include <expected>
#include <string_view>

namespace sim::data {

[[nodiscard]] std::expected<uint32_t, LoadError>
load_archetypes_from_string(std::string_view toml_text,
                            const AbilityRegistry& abilities,
                            EntityArchetypeRegistry& registry);

[[nodiscard]] std::expected<uint32_t, LoadError> load_archetypes_from_file(
    std::string_view path, const AbilityRegistry& abilities, EntityArchetypeRegistry& registry);

}  // namespace sim::data