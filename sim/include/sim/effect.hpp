#pragma once

#include "sim/sim_float.hpp"
#include "sim/stat_id.hpp"

#include <cstdint>
#include <variant>

namespace sim {

enum class StatusType : uint8_t;

using AbilityId = uint16_t;

inline constexpr AbilityId NoAbility = 0;

struct DamageEffect {
    StatId damage_stat = StatId::None;
    SimFloat base_amount = 0.0f;
};

struct ApplyStatusEffect {
    StatusType status = {};
    SimFloat chance = 1.0f;
    SimFloat magnitude = 0.0f;
    uint32_t duration_ticks = 0;
};

struct ChainEffect {
    uint8_t max_chains = 0;
    SimFloat falloff = 1.0f;
    SimFloat radius = 0.0f;
};

struct TriggerEffect {
    AbilityId ability_to_trigger = NoAbility;
    SimFloat chance = 1.0f;
};

using Effect = std::variant<DamageEffect, ApplyStatusEffect, ChainEffect, TriggerEffect>;

}  // namespace sim