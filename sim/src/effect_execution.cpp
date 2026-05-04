#include "sim/effect_execution.hpp"

#include "sim/ability_definition.hpp"
#include "sim/effect.hpp"
#include "sim/effect_context.hpp"
#include "sim/eval_context.hpp"
#include "sim/evaluate.hpp"
#include "sim/sim_commands.hpp"
#include "sim/world.hpp"

namespace sim {

void execute_damage_effect(const DamageEffect& effect, EffectContext& ctx) {
    if (ctx.world == nullptr || ctx.commands == nullptr) {
        return;
    }
    if (ctx.instance == nullptr || ctx.definition == nullptr) {
        return;
    }
    if (!ctx.world->is_alive(ctx.current_target)) {
        return;
    }
    if (ctx.caster_stats == nullptr) {
        return;
    }

    const EvalContext eval{
        .attacker = ctx.instance->caster,
        .target = ctx.current_target,
        .ability_tags = ctx.definition->tags,
        .target_tags = tags::None,
        .current_tick = ctx.current_tick,
    };

    const SimFloat from_modifiers =
        evaluate_stat(*ctx.world, *ctx.caster_stats, effect.damage_stat, eval);
    const SimFloat total = effect.base_amount + from_modifiers;

    if (total <= 0.0f) {
        return;
    }

    ctx.commands->deal_damage.push_back(DealDamageCommand{
        .attacker = ctx.instance->caster,
        .target = ctx.current_target,
        .amount = total,
        .ability_tags = ctx.definition->tags,
    });
}

}  // namespace sim