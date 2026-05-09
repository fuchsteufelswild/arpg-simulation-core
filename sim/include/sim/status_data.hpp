#pragma once

#include "sim/sim_float.hpp"

#include <variant>

namespace sim {

struct ChillData {
    SimFloat slow_amount = 0.0f;
};

struct IgniteData {
    SimFloat damage_per_tick = 0.0f;
};

struct PoisonData {
    SimFloat damage_per_tick = 0.0f;
};

struct StunData {};

using StatusPayload = std::variant<ChillData, IgniteData, PoisonData, StunData>;

}  // namespace sim