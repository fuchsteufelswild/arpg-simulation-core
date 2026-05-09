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
                       [&](const TriggerEffect&) {},
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

    std::vector<AbilityInstance> to_resolve;
    const uint64_t current_tick = res.current_tick;

    for (const AbilityInstance& inst : casting_) {
        if (current_tick >= inst.resolve_tick) {
            to_resolve.push_back(inst);
        }
    }

    std::erase_if(casting_, [current_tick](const AbilityInstance& inst) {
        return current_tick >= inst.resolve_tick;
    });

    for (const AbilityInstance& inst : to_resolve) {
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