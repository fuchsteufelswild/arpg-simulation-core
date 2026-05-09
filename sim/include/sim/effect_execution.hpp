#pragma once

#include "sim/effect.hpp"
namespace sim {

struct DamageEffect;
struct ApplyStatusEffect;
struct EffectContext;
struct ChainEffect;

void execute_damage_effect(const DamageEffect& effect, EffectContext& ctx);
void execute_apply_status_effect(const ApplyStatusEffect& effect, EffectContext& ctx);
void execute_chain_effect(const ChainEffect& effect, EffectContext& ctx);

}  // namespace sim