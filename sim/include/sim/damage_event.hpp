#pragma once

#include "sim/entity_handle.hpp"
#include "sim/sim_float.hpp"
#include "sim/tags.hpp"

namespace sim {

struct DamageEvent {
    EntityHandle target;
    EntityHandle attacker;
    SimFloat amount = 0.0f;
    TagMask ability_tags = tags::None;
    bool was_crit = false;
};

}  // namespace sim