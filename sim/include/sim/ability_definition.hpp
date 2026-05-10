#pragma once

#include "sim/effect.hpp"
#include "sim/sim_float.hpp"
#include "sim/tags.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace sim {

struct AbilityDefinition {
    AbilityId id = NoAbility;
    std::string name;
    TagMask tags = tags::None;

    uint32_t cast_time_ticks = 0;
    uint32_t cooldown_ticks = 0;
    SimFloat travel_speed = 0.0f;

    std::vector<Effect> effects;
};

}  // namespace sim