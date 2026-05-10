#include "sim/ability_system.hpp"

#include "sim/ability_registry.hpp"
#include "sim/effect.hpp"
#include "sim/effect_context.hpp"
#include "sim/effect_execution.hpp"
#include "sim/entity_stats.hpp"
#include "sim/spatial_grid.hpp"
#include "sim/util/overload.hpp"
#include "sim/world.hpp"

#include <variant>

namespace sim {

namespace {

[[nodiscard]] AbilityPhase initial_phase(const AbilityDefinition& def) noexcept {
    if (def.cast_time_ticks > 0) {
        return AbilityPhase::Casting;
    }
    if (def.travel_speed > 0.0f) {
        return AbilityPhase::Traveling;
    }
    return AbilityPhase::Resolving;
}

void resolve_instance(const AbilityInstance& instance,
                      const AbilityDefinition& def,
                      const ResolutionContext& res) {
    if (res.world == nullptr || res.commands == nullptr) {
        return;
    }

    if (!res.world->is_alive(instance.caster)) {
        return;
    }
    const EntityStats& caster_stats = res.world->stats(instance.caster);

    EffectContext ctx{
        .definition = &def,
        .instance = &instance,
        .current_target = instance.initial_target,
        .world = res.world,
        .caster_stats = &caster_stats,
        .target_stats = nullptr,
        .commands = res.commands,
        .current_tick = res.current_tick,
        .grid = res.grid,
    };

    if (res.world->is_alive(instance.initial_target)) {
        ctx.target_stats = &res.world->stats(instance.initial_target);
    }

    for (const Effect& effect : def.effects) {
        std::visit(util::Overload{
                       [&](const DamageEffect& d) { execute_damage_effect(d, ctx); },
                       [&](const ApplyStatusEffect& s) { execute_apply_status_effect(s, ctx); },
                       [&](const ChainEffect& c) { execute_chain_effect(c, ctx); },
                       [&](const TriggerEffect& t) { execute_trigger_effect(t, ctx); },
                   },
                   effect);
    }
}

}  // namespace

void AbilitySystem::add(AbilityInstance instance) {
    switch (instance.phase) {
    case AbilityPhase::Casting:
        casting_.push_back(instance);
        break;
    case AbilityPhase::Traveling:
        traveling_.push_back(instance);
        break;
    case AbilityPhase::Resolving:
        resolving_.push_back(instance);
        break;
    case AbilityPhase::Finished:
        break;
    }
}

void AbilitySystem::cast(AbilityId ability_id,
                         EntityHandle caster,
                         EntityHandle target,
                         SimFloat target_x,
                         SimFloat target_y,
                         const ResolutionContext& res) {
    if (res.registry == nullptr || res.world == nullptr) {
        return;
    }
    const AbilityDefinition* def = res.registry->find(ability_id);
    if (def == nullptr) {
        return;
    }
    if (!res.world->is_alive(caster)) {
        return;
    }

    if (def->cooldown_ticks > 0) {
        if (!res.world->cooldowns(caster).is_ready(ability_id, res.current_tick)) {
            return;
        }
    }

    const Transform& origin = res.world->transform(caster);

    AbilityInstance instance{
        .definition_id = ability_id,
        .phase = initial_phase(*def),
        .caster = caster,
        .initial_target = target,
        .start_tick = res.current_tick,
        .resolve_tick = res.current_tick + def->cast_time_ticks,
        .origin_x = origin.x,
        .origin_y = origin.y,
        .target_x = target_x,
        .target_y = target_y,
        .current_x = origin.x,
        .current_y = origin.y,
    };

    if (def->cooldown_ticks > 0) {
        res.world->cooldowns(caster).set(
            ability_id, res.current_tick + def->cast_time_ticks + def->cooldown_ticks);
    }

    if (instance.phase == AbilityPhase::Resolving) {
        resolve_instance(instance, *def, res);
        return;
    }

    add(instance);
}

void AbilitySystem::update(const ResolutionContext& res) {
    if (res.registry == nullptr) {
        return;
    }

    advance_casting(res);
    advance_traveling(res);
}

void AbilitySystem::advance_casting(const ResolutionContext& res) {
    const uint64_t current_tick = res.current_tick;

    std::vector<AbilityInstance> ready;
    for (const AbilityInstance& inst : casting_) {
        if (current_tick >= inst.resolve_tick) {
            ready.push_back(inst);
        }
    }
    std::erase_if(casting_, [current_tick](const AbilityInstance& inst) {
        return current_tick >= inst.resolve_tick;
    });

    for (AbilityInstance& inst : ready) {
        const AbilityDefinition* def = res.registry->find(inst.definition_id);
        if (def == nullptr) {
            continue;
        }
        if (def->travel_speed > 0.0f) {
            inst.phase = AbilityPhase::Traveling;
            traveling_.push_back(inst);
        } else {
            resolve_instance(inst, *def, res);
        }
    }
}

void AbilitySystem::advance_traveling(const ResolutionContext& res) {
    std::vector<AbilityInstance> arrived;
    std::vector<AbilityInstance> still_flying;
    still_flying.reserve(traveling_.size());

    for (AbilityInstance& inst : traveling_) {
        const AbilityDefinition* def = res.registry->find(inst.definition_id);
        if (def == nullptr) {
            continue;
        }

        const SimFloat dx = inst.target_x - inst.current_x;
        const SimFloat dy = inst.target_y - inst.current_y;
        const SimFloat dist_sq = (dx * dx) + (dy * dy);
        const SimFloat step_sq = def->travel_speed * def->travel_speed;

        if (dist_sq <= step_sq) {
            inst.current_x = inst.target_x;
            inst.current_y = inst.target_y;
            arrived.push_back(inst);
        } else {
            const SimFloat dist = std::sqrt(dist_sq);
            inst.current_x += dx / dist * def->travel_speed;
            inst.current_y += dy / dist * def->travel_speed;
            still_flying.push_back(inst);
        }
    }

    traveling_ = std::move(still_flying);

    for (const AbilityInstance& inst : arrived) {
        const AbilityDefinition* def = res.registry->find(inst.definition_id);
        if (def == nullptr) {
            continue;
        }
        resolve_instance(inst, *def, res);
    }
}

void AbilitySystem::clear() noexcept {
    casting_.clear();
    traveling_.clear();
    resolving_.clear();
}

}  // namespace sim