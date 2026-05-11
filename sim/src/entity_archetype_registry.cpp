#include "sim/entity_archetype_registry.hpp"

#include <cassert>

namespace sim {

ArchetypeId EntityArchetypeRegistry::register_archetype(EntityArchetype archetype) {
    const auto new_id = static_cast<ArchetypeId>(archetypes_.size() + 1);
    archetype.id = new_id;
    archetypes_.push_back(std::move(archetype));
    return new_id;
}

ArchetypeId EntityArchetypeRegistry::find_by_name(std::string_view name) const noexcept {
    for (const auto& archetype : archetypes_) {
        if (archetype.name == name) {
            return archetype.id;
        }
    }
    return NoArchetype;
}

const EntityArchetype& EntityArchetypeRegistry::get(ArchetypeId id) const noexcept {
    assert(id > 0 && id <= archetypes_.size() && "EntityArchetypeRegistry::get out of range");
    return archetypes_[id - 1];
}

EntityHandle EntityArchetypeRegistry::spawn(World& world, ArchetypeId id) const {
    const EntityArchetype& archetype = get(id);
    const EntityHandle handle = world.spawn(archetype.kind);

    world.health(handle) = Health{
        .current = archetype.max_health,
        .max = archetype.max_health,
    };

    if (archetype.ai.has_value()) {
        const AIArchetype& ai = *archetype.ai;
        world.ai_behaviour(handle) = AIBehaviour{
            .state = AIState::Idle,
            .preferred_ability = ai.preferred_ability,
            .attack_range = ai.attack_range,
            .detection_range = ai.detection_range,
            .leash_range = ai.leash_range,
            .move_speed = ai.move_speed,
        };
    }

    return handle;
}

}  // namespace sim