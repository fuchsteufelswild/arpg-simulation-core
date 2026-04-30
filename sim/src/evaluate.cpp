#include "sim/evaluate.hpp"

#include "sim/conditions.hpp"
#include "sim/entity_stats.hpp"
#include "sim/modifier.hpp"
#include "sim/world.hpp"

namespace sim {

namespace {

[[nodiscard]] bool
modifier_applies(const Modifier& mod, const World& world, const EvalContext& ctx) noexcept {
    if (!tags_match(mod.required_tags, ctx.ability_tags)) {
        return false;
    }
    if (!evaluate_condition(mod.condition, mod.condition_param, world, ctx)) {
        return false;
    }
    return true;
}

struct AccumulatedTerms {
    SimFloat base = 0.0f;
    SimFloat increased_sum = 0.0f;
    SimFloat more_product = 1.0f;
};

void apply_modifier(AccumulatedTerms& terms, const Modifier& mod) noexcept {
    switch (mod.op) {
    case ModOp::Flat:
        terms.base += mod.magnitude;
        break;
    case ModOp::Increased:
        terms.increased_sum += mod.magnitude;
        break;
    case ModOp::More:
        terms.more_product *= (1.0f + mod.magnitude);
        break;
    }
}

}  // namespace

SimFloat evaluate_stat(const World& world,
                       const EntityStats& stats,
                       StatId stat,
                       const EvalContext& ctx) noexcept {
    AccumulatedTerms terms;

    for (const Modifier& mod : stats.primary_modifiers(stat)) {
        if (!modifier_applies(mod, world, ctx)) {
            continue;
        }
        apply_modifier(terms, mod);
    }

    return terms.base * (1.0f + terms.increased_sum) * terms.more_product;
}

}  // namespace sim