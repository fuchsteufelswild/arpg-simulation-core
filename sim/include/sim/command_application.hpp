#pragma once

#include "sim/damage_event.hpp"

namespace sim {

struct ResolutionContext;
class AbilitySystem;

void apply_commands(AbilitySystem& abilities,
                    const ResolutionContext& res,
                    std::vector<DamageEvent>* events_out);

}  // namespace sim