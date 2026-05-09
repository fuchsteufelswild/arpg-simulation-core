#pragma once

#include "sim/entity_handle.hpp"
#include "sim/modifier_kinds.hpp"
#include "sim/status_data.hpp"
#include "sim/status_type.hpp"

#include <cstdint>

namespace sim {

struct StatusInstance {
    StatusType type = StatusType::None;
    SourceId source = 0;
    EntityHandle applier = {};
    uint64_t apply_tick = 0;
    uint64_t expire_tick = 0;
    uint16_t stack_count = 1;
    uint64_t last_tick_processed = 0;

    StatusPayload payload;
};

}  // namespace sim