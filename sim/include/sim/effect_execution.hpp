#pragma once

namespace sim {

struct DamageEffect;
struct EffectContext;

void execute_damage_effect(const DamageEffect& effect, EffectContext& ctx);

}  // namespace sim