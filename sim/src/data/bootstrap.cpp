#include "sim/data/bootstrap.hpp"

#include "sim/data/ability_loader.hpp"
#include "sim/data/archetype_loader.hpp"
#include "sim/sim.hpp"

#include <filesystem>

namespace sim::data {

namespace {

std::filesystem::path get_data_dir() {
    auto exe = std::filesystem::current_path();
    return exe / "data";
}

}  // namespace

std::expected<void, LoadError> bootstrap(Sim& sim) {
    auto abilities_path = get_data_dir() / "abilities.toml";
    auto abilities_result =
        load_abilities_from_file(abilities_path.string(), sim.ability_registry());
    if (!abilities_result) {
        return std::unexpected(abilities_result.error());
    }

    auto archetypes_path = get_data_dir() / "archetypes.toml";
    auto archetypes_result = load_archetypes_from_file(
        archetypes_path.string(), sim.ability_registry(), sim.archetype_registry());
    if (!archetypes_result) {
        return std::unexpected(archetypes_result.error());
    }
    return {};
}

std::expected<void, LoadError> bootstrap(Sim& sim, const ContentPaths& paths) {
    auto abilities_result = load_abilities_from_file(paths.abilities_path, sim.ability_registry());
    if (!abilities_result) {
        return std::unexpected(abilities_result.error());
    }
    auto archetypes_result = load_archetypes_from_file(
        paths.archetypes_path, sim.ability_registry(), sim.archetype_registry());
    if (!archetypes_result) {
        return std::unexpected(archetypes_result.error());
    }
    return {};
}

std::expected<void, LoadError> bootstrap_from_strings(Sim& sim, const ContentSources& sources) {
    auto abilities_result =
        load_abilities_from_string(sources.abilities_toml, sim.ability_registry());
    if (!abilities_result) {
        return std::unexpected(abilities_result.error());
    }
    auto archetypes_result = load_archetypes_from_string(
        sources.archetypes_toml, sim.ability_registry(), sim.archetype_registry());
    if (!archetypes_result) {
        return std::unexpected(archetypes_result.error());
    }
    return {};
}

std::expected<EntityHandle, LoadError>
spawn_archetype_by_name(Sim& sim, std::string_view archetype_name, SimFloat x, SimFloat y) {
    const ArchetypeId id = sim.archetype_registry().find_by_name(archetype_name);
    if (id == NoArchetype) {
        return std::unexpected(LoadError{"archetype not found: " + std::string{archetype_name}});
    }

    const EntityHandle handle = sim.archetype_registry().spawn(sim.world(), id);
    sim.world().transform(handle) = Transform{.x = x, .y = y, .facing_radians = 0.0f};
    return handle;
}

}  // namespace sim::data