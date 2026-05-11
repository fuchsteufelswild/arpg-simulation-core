#pragma once

#include "sim/effect.hpp"
#include "sim/sim_float.hpp"
#include "sim/world.hpp"

#include <optional>
#include <string>

namespace sim {

using ArchetypeId = uint16_t;
inline constexpr ArchetypeId NoArchetype = 0;

struct AIArchetype {
    AbilityId preferred_ability = NoAbility;
    SimFloat attack_range = 0.0f;
    SimFloat detection_range = 0.0f;
    SimFloat leash_range = 0.0f;
    SimFloat move_speed = 0.0f;
};

struct EntityArchetype {
    ArchetypeId id = NoArchetype;
    std::string name;
    EntityKind kind = EntityKind::None;
    SimFloat max_health = 0.0f;
    std::optional<AIArchetype> ai;
};

}  // namespace sim