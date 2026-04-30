#include "sim/conditions.hpp"

#include "sim/recent_events.hpp"
#include "sim/world.hpp"

namespace sim {

namespace {

[[nodiscard]] bool while_at_full_life(const World& world, const EvalContext& ctx) noexcept {
    if (!world.is_alive(ctx.attacker)) {
        return false;
    }
    const Health& h = world.health(ctx.attacker);
    return h.max > 0.0f && h.current >= h.max;
}

[[nodiscard]] bool
while_at_low_life(const World& world, const EvalContext& ctx, SimFloat threshold) noexcept {
    if (!world.is_alive(ctx.attacker)) {
        return false;
    }
    const Health& h = world.health(ctx.attacker);
    if (h.max <= 0.0f) {
        return false;
    }
    return (h.current / h.max) <= threshold;
}

[[nodiscard]] bool against_status(const EvalContext& ctx, TagMask status_tag) noexcept {
    return (ctx.target_tags & status_tag) != 0;
}

[[nodiscard]] bool
crit_recently(const World& world, const EvalContext& ctx, SimFloat window_seconds) noexcept {
    if (!world.is_alive(ctx.attacker)) {
        return false;
    }
    constexpr SimFloat tick_rate = 30.0f;
    const auto window_ticks = static_cast<uint64_t>(window_seconds * tick_rate);
    return world.recent_events(ctx.attacker)
        .any_within(EventType::CritHit, ctx.current_tick, window_ticks);
}

}  // namespace

bool evaluate_condition(ConditionId condition,
                        SimFloat condition_param,
                        const World& world,
                        const EvalContext& ctx) noexcept {
    switch (condition) {
    case ConditionId::None:
        return true;
    case ConditionId::WhileAtFullLife:
        return while_at_full_life(world, ctx);
    case ConditionId::WhileAtLowLife:
        return while_at_low_life(world, ctx, condition_param);
    case ConditionId::AgainstChilled:
        return against_status(ctx, tags::Chilled);
    case ConditionId::AgainstIgnited:
        return against_status(ctx, tags::Ignited);
    case ConditionId::OnCrit:
        // Implement when Hit data is available
        return false;
    case ConditionId::CritRecently:
        return crit_recently(world, ctx, condition_param);
    }
    return false;
}

}  // namespace sim