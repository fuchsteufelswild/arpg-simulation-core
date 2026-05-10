#include "sim/effect_execution.hpp"

#include "sim/ability_definition.hpp"
#include "sim/effect.hpp"
#include "sim/effect_context.hpp"
#include "sim/eval_context.hpp"
#include "sim/evaluate.hpp"
#include "sim/sim_commands.hpp"
#include "sim/sim_rng.hpp"
#include "sim/spatial_grid.hpp"
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
        .is_crit = false,
    };

    const SimFloat from_modifiers =
        evaluate_stat(*ctx.world, *ctx.caster_stats, effect.damage_stat, eval);
    SimFloat amount = effect.base_amount + from_modifiers;

    if (amount <= 0.0f) {
        return;
    }

    bool crit = false;
    if (ctx.rng != nullptr) {
        const SimFloat crit_chance =
            evaluate_stat(*ctx.world, *ctx.caster_stats, StatId::CritChance, eval);
        if (crit_chance > 0.0f && ctx.rng->next_float_unit() < crit_chance) {
            crit = true;

            const EvalContext crit_eval{
                .attacker = ctx.instance->caster,
                .target = ctx.current_target,
                .ability_tags = ctx.definition->tags,
                .target_tags = target_tags,
                .current_tick = ctx.current_tick,
                .is_crit = true,
            };

            const SimFloat from_crit_mods =
                evaluate_stat(*ctx.world, *ctx.caster_stats, effect.damage_stat, crit_eval);
            amount = effect.base_amount + from_crit_mods;

            const SimFloat crit_mult =
                evaluate_stat(*ctx.world, *ctx.caster_stats, StatId::CritMultiplier, eval);
            const SimFloat effective_mult = (crit_mult > 0.0f) ? crit_mult : 1.5f;
            amount *= effective_mult;
        }
    }

    if (amount <= 0.0f) {
        return;
    }

    if (crit && ctx.world->is_alive(ctx.instance->caster)) {
        ctx.world->recent_events(ctx.instance->caster).push(EventType::CritHit, ctx.current_tick);
    }

    ctx.commands->deal_damage.push_back(DealDamageCommand{
        .attacker = ctx.instance->caster,
        .target = ctx.current_target,
        .amount = amount,
        .ability_tags = ctx.definition->tags,
        .was_crit = crit,
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

void execute_chain_effect(const ChainEffect& effect, EffectContext& ctx) {
    if (ctx.world == nullptr || ctx.commands == nullptr || ctx.grid == nullptr) {
        return;
    }
    if (ctx.instance == nullptr || ctx.definition == nullptr || ctx.caster_stats == nullptr) {
        return;
    }
    if (effect.max_chains == 0 || effect.radius <= 0.0f) {
        return;
    }
    if (!ctx.world->is_alive(ctx.current_target)) {
        return;
    }

    std::vector<EntityHandle> hit_targets;
    hit_targets.push_back(ctx.current_target);

    EntityHandle chain_origin = ctx.current_target;
    SimFloat damage_multiplier = effect.falloff;

    for (uint8_t hop = 0; hop < effect.max_chains; ++hop) {
        if (!ctx.world->is_alive(chain_origin)) {
            break;
        }

        const Transform& origin_transform = ctx.world->transform(chain_origin);
        const std::vector<EntityHandle> candidates =
            ctx.grid->query_within(origin_transform.x, origin_transform.y, effect.radius);

        EntityHandle next_target;
        for (const EntityHandle& candidate : candidates) {
            if (candidate == ctx.instance->caster) {
                continue;
            }
            if (!ctx.world->is_alive(candidate)) {
                continue;
            }
            const auto already_hit = std::ranges::find(hit_targets, candidate);
            if (already_hit != hit_targets.end()) {
                continue;
            }
            next_target = candidate;
            break;
        }

        if (next_target.is_null()) {
            break;
        }

        for (const Effect& other : ctx.definition->effects) {
            if (!std::holds_alternative<DamageEffect>(other)) {
                continue;
            }
            const auto& dmg = std::get<DamageEffect>(other);

            const EvalContext eval{
                .attacker = ctx.instance->caster,
                .target = next_target,
                .ability_tags = ctx.definition->tags,
                .target_tags = ctx.world->status_list(next_target).combined_tags(),
                .current_tick = ctx.current_tick,
            };

            const SimFloat from_modifiers =
                evaluate_stat(*ctx.world, *ctx.caster_stats, dmg.damage_stat, eval);
            const SimFloat total = (dmg.base_amount + from_modifiers) * damage_multiplier;

            if (total <= 0.0f) {
                continue;
            }

            ctx.commands->deal_damage.push_back(DealDamageCommand{
                .attacker = ctx.instance->caster,
                .target = next_target,
                .amount = total,
                .ability_tags = ctx.definition->tags,
            });
        }

        hit_targets.push_back(next_target);
        chain_origin = next_target;
        damage_multiplier *= effect.falloff;
    }
}

void execute_trigger_effect(const TriggerEffect& effect, EffectContext& ctx) {
    if (ctx.commands == nullptr || ctx.instance == nullptr) {
        return;
    }
    if (effect.ability_to_trigger == NoAbility) {
        return;
    }
    if (effect.chance < 1.0f) {
        return;
    }

    SimFloat target_x = ctx.instance->target_x;
    SimFloat target_y = ctx.instance->target_y;

    if (ctx.world != nullptr && ctx.world->is_alive(ctx.current_target)) {
        const Transform& t = ctx.world->transform(ctx.current_target);
        target_x = t.x;
        target_y = t.y;
    }

    ctx.commands->cast_ability.push_back(CastAbilityCommand{
        .ability_id = effect.ability_to_trigger,
        .caster = ctx.instance->caster,
        .target = ctx.current_target,
        .target_x = target_x,
        .target_y = target_y,
    });
}

}  // namespace sim