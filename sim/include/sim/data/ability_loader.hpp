#pragma once

#include "sim/ability_registry.hpp"

#include <expected>
#include <string>
#include <string_view>

namespace sim::data {

struct LoadError {
    std::string message;
};

[[nodiscard]] std::expected<uint32_t, LoadError>
load_abilities_from_string(std::string_view toml_text, AbilityRegistry& registry);

[[nodiscard]] std::expected<uint32_t, LoadError>
load_abilities_from_file(std::string_view path, AbilityRegistry& registry);

}  // namespace sim::data