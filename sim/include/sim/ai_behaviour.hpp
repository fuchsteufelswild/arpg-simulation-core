#pragma once

#include "sim/effect.hpp"
#include "sim/entity_handle.hpp"
#include "sim/sim_float.hpp"

#include <cstdint>

namespace sim {

enum class AIState : uint8_t {
    Inactive = 0,
    Idle,
    Chase,
    Attack,
};

struct AIBehaviour {
    AIState state = AIState::Inactive;

    AbilityId preferred_ability = NoAbility;
    SimFloat attack_range = 0.0f;
    SimFloat detection_range = 0.0f;
    SimFloat leash_range = 0.0f;
    SimFloat move_speed = 0.0f;

    EntityHandle target{};
    uint64_t next_cast_attempt_tick = 0;
    uint64_t next_scan_tick = 0;
};

}  // namespace sim