#pragma once

#include "sim/effect.hpp"
#include "sim/entity_handle.hpp"
#include "sim/sim_float.hpp"

#include <cstdint>

namespace sim {

enum class AbilityPhase : uint8_t {
    Casting = 0,
    Traveling,
    Resolving,
    Finished,
};

struct AbilityInstance {
    AbilityId definition_id = NoAbility;
    AbilityPhase phase = AbilityPhase::Casting;

    EntityHandle caster;
    EntityHandle initial_target;

    uint64_t start_tick = 0;
    uint64_t resolve_tick = 0;

    SimFloat origin_x = 0.0f;
    SimFloat origin_y = 0.0f;
    SimFloat target_x = 0.0f;
    SimFloat target_y = 0.0f;

    SimFloat current_x = 0.0f;
    SimFloat current_y = 0.0f;

    uint8_t chains_remaining = 0;
};

}  // namespace sim