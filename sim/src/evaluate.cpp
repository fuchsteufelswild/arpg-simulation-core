#include "sim/evaluate.hpp"

#include "sim/conditions.hpp"
#include "sim/entity_stats.hpp"
#include "sim/modifier.hpp"
#include "sim/world.hpp"

namespace sim {

namespace {

[[nodiscard]] bool
primary_applies(const Modifier& mod, const World& world, const EvalContext& ctx) noexcept {
    if (!tags_match(mod.required_tags, ctx.ability_tags)) {
        return false;
    }
    if (!evaluate_condition(mod.condition, mod.condition_param, world, ctx)) {
        return false;
    }
    return true;
}

[[nodiscard]] bool meta_amplifies(const Modifier& meta,
                                  const Modifier& primary,
                                  const World& world,
                                  const EvalContext& ctx) noexcept {
    if (!tags_match(meta.required_tags, primary.required_tags)) {
        return false;
    }
    if (!evaluate_condition(meta.condition, meta.condition_param, world, ctx)) {
        return false;
    }
    return true;
}

[[nodiscard]] SimFloat amplification_for(const Modifier& primary,
                                         std::span<const Modifier> meta_mods,
                                         const World& world,
                                         const EvalContext& ctx) noexcept {
    SimFloat amplification = 1.0f;
    for (const Modifier& meta : meta_mods) {
        if (meta_amplifies(meta, primary, world, ctx)) {
            amplification *= (1.0f + meta.magnitude);
        }
    }
    return amplification;
}

struct AccumulatedTerms {
    SimFloat base = 0.0f;
    SimFloat increased_sum = 0.0f;
    SimFloat more_product = 1.0f;
};

void apply_modifier(AccumulatedTerms& terms, const Modifier& mod, SimFloat amplification) noexcept {
    const SimFloat effective = mod.magnitude * amplification;
    switch (mod.op) {
    case ModOp::Flat:
        terms.base += effective;
        break;
    case ModOp::Increased:
        terms.increased_sum += effective;
        break;
    case ModOp::More:
        terms.more_product *= (1.0f + effective);
        break;
    }
}

}  // namespace

SimFloat evaluate_stat(const World& world,
                       const EntityStats& stats,
                       StatId stat,
                       const EvalContext& ctx) noexcept {
    AccumulatedTerms terms;
    const auto meta_mods = stats.meta_modifiers();

    for (const Modifier& mod : stats.primary_modifiers(stat)) {
        if (!primary_applies(mod, world, ctx)) {
            continue;
        }
        const SimFloat amplification = amplification_for(mod, meta_mods, world, ctx);
        apply_modifier(terms, mod, amplification);
    }

    return terms.base * (1.0f + terms.increased_sum) * terms.more_product;
}

}  // namespace sim