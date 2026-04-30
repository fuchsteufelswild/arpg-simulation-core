#pragma once

#include "sim/entity_handle.hpp"
#include "sim/tags.hpp"

#include <cstdint>

namespace sim {

struct EvalContext {
    EntityHandle attacker = {};
    EntityHandle target = {};
    TagMask ability_tags = tags::None;
    TagMask target_tags = tags::None;
    uint64_t current_tick = 0;
};

}  // namespace sim