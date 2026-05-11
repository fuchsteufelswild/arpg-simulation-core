#pragma once

#include "sim/entity_archetype.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace sim {

class EntityArchetypeRegistry {
public:
    ArchetypeId register_archetype(EntityArchetype archetype);

    [[nodiscard]] ArchetypeId find_by_name(std::string_view name) const noexcept;
    [[nodiscard]] const EntityArchetype& get(ArchetypeId id) const noexcept;

    [[nodiscard]] uint32_t size() const noexcept {
        return static_cast<uint32_t>(archetypes_.size());
    }

    [[nodiscard]] EntityHandle spawn(World& world, ArchetypeId id) const;

private:
    std::vector<EntityArchetype> archetypes_;
};

}  // namespace sim