#pragma once

#include "sim/modifier_kinds.hpp"
#include "sim/sim_float.hpp"
#include "sim/stat_id.hpp"
#include "sim/tags.hpp"

#include <cstdint>
#include <type_traits>

namespace sim {

struct Modifier {
    StatId stat = StatId::None;
    ModOp op = ModOp::Flat;
    ModPhase phase = ModPhase::Primary;
    ConditionId condition = ConditionId::None;
    TagMask required_tags = tags::None;
    SimFloat magnitude = 0.0f;
    SimFloat condition_param = 0.0f;
    SourceId source = 0;
    uint16_t _pad = 0;  // NOLINT(readability-identifier-naming)
};

static_assert(std::is_trivially_copyable_v<Modifier>);
static_assert(std::is_standard_layout_v<Modifier>);
static_assert(sizeof(Modifier) == 32);
static_assert(alignof(Modifier) == 8);

}  // namespace sim