#pragma once

#include "sim/eval_context.hpp"
#include "sim/modifier_kinds.hpp"
#include "sim/sim_float.hpp"

namespace sim {

class World;

[[nodiscard]] bool evaluate_condition(ConditionId condition,
                                      SimFloat condition_param,
                                      const World& world,
                                      const EvalContext& ctx) noexcept;

}  // namespace sim