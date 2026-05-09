#pragma once

namespace sim {

struct ResolutionContext;
class AbilitySystem;

void apply_commands(AbilitySystem& abilities, const ResolutionContext& res);

}  // namespace sim