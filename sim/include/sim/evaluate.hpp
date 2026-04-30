#pragma once

#include "sim/eval_context.hpp"
#include "sim/sim_float.hpp"
#include "sim/stat_id.hpp"

namespace sim {

class EntityStats;

[[nodiscard]] SimFloat
evaluate_stat(const EntityStats& stats, StatId stat, const EvalContext& ctx) noexcept;

}  // namespace sim