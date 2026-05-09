#pragma once

namespace sim {

struct DamageEffect;
struct ApplyStatusEffect;
struct EffectContext;

void execute_damage_effect(const DamageEffect& effect, EffectContext& ctx);
void execute_apply_status_effect(const ApplyStatusEffect& effect, EffectContext& ctx);

}  // namespace sim