#include "sim/effect_execution.hpp"

#include "sim/ability_definition.hpp"
#include "sim/effect.hpp"
#include "sim/effect_context.hpp"
#include "sim/eval_context.hpp"
#include "sim/evaluate.hpp"
#include "sim/sim_commands.hpp"
#include "sim/world.hpp"

namespace sim {

namespace {

[[nodiscard]] StatusPayload payload_for(StatusType type, SimFloat magnitude) noexcept {
    switch (type) {
    case StatusType::Chill:
        return ChillData{.slow_amount = magnitude};
    case StatusType::Ignite:
        return IgniteData{.damage_per_tick = magnitude};
    case StatusType::Poison:
        return PoisonData{.damage_per_tick = magnitude};
    case StatusType::Stun:
        return StunData{};
    case StatusType::None:
        return ChillData{};
    }
    return ChillData{};
}

}  // namespace

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

    const TagMask target_tags = (ctx.world != nullptr && ctx.world->is_alive(ctx.current_target))
                                    ? ctx.world->status_list(ctx.current_target).combined_tags()
                                    : tags::None;

    const EvalContext eval{
        .attacker = ctx.instance->caster,
        .target = ctx.current_target,
        .ability_tags = ctx.definition->tags,
        .target_tags = target_tags,
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

void execute_apply_status_effect(const ApplyStatusEffect& effect, EffectContext& ctx) {
    if (ctx.world == nullptr || ctx.instance == nullptr) {
        return;
    }
    if (!ctx.world->is_alive(ctx.current_target)) {
        return;
    }
    if (effect.status == StatusType::None) {
        return;
    }

    StatusInstance status{
        .type = effect.status,
        .source = make_source_id(source_categories::Status, static_cast<uint16_t>(effect.status)),
        .applier = ctx.instance->caster,
        .apply_tick = ctx.current_tick,
        .expire_tick = ctx.current_tick + effect.duration_ticks,
        .last_tick_processed = ctx.current_tick,
        .payload = payload_for(effect.status, effect.magnitude),
    };

    ctx.world->status_list(ctx.current_target).add(status);
}

}  // namespace sim