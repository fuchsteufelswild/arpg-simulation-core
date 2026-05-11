#pragma once

#include "sim/data/ability_loader.hpp"
#include "sim/entity_handle.hpp"

#include <expected>
#include <string_view>

namespace sim {
class Sim;
}

namespace sim::data {

struct ContentPaths {
    std::string_view abilities_path;
    std::string_view archetypes_path;
};

struct ContentSources {
    std::string_view abilities_toml;
    std::string_view archetypes_toml;
};

[[nodiscard]] std::expected<void, LoadError> bootstrap(Sim& sim);

[[nodiscard]] std::expected<void, LoadError> bootstrap(Sim& sim, const ContentPaths& paths);

[[nodiscard]] std::expected<void, LoadError> bootstrap_from_strings(Sim& sim,
                                                                    const ContentSources& sources);

[[nodiscard]] std::expected<EntityHandle, LoadError>
spawn_archetype_by_name(Sim& sim, std::string_view archetype_name, SimFloat x, SimFloat y);

}  // namespace sim::data