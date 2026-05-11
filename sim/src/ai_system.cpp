#include "sim/ai_system.hpp"

#include "sim/ai_behaviour.hpp"
#include "sim/sim_commands.hpp"
#include "sim/sim_float.hpp"
#include "sim/spatial_grid.hpp"
#include "sim/world.hpp"

#include <cmath>
#include <vector>

namespace sim {

namespace {

constexpr std::uint64_t KCastAttemptCooldownTicks = 15;  // ~0.5s at 30Hz
constexpr std::uint64_t KIdleRescanCooldownTicks = 6;    // ~0.2s at 30Hz

[[nodiscard]] SimFloat distance_squared(const Transform& a, const Transform& b) noexcept {
    const SimFloat dx = a.x - b.x;
    const SimFloat dy = a.y - b.y;
    return (dx * dx) + (dy * dy);
}

[[nodiscard]] bool is_hostile(EntityKind self_kind, EntityKind target_kind) noexcept {
    if (self_kind == EntityKind::Enemy) {
        return target_kind == EntityKind::Player;
    }
    if (self_kind == EntityKind::Player) {
        return target_kind == EntityKind::Enemy;
    }
    return false;
}

[[nodiscard]] EntityHandle find_nearest_hostile(const World& world,
                                                const SpatialGrid& grid,
                                                EntityHandle self,
                                                const Transform& origin,
                                                SimFloat radius) {
    const EntityKind self_kind = world.kind(self);
    const std::vector<EntityHandle> candidates = grid.query_within(origin.x, origin.y, radius);

    for (EntityHandle candidate : candidates) {
        if (candidate.index == self.index) {
            continue;
        }
        if (!world.is_alive(candidate)) {
            continue;
        }
        if (!is_hostile(self_kind, world.kind(candidate))) {
            continue;
        }
        if (world.health(candidate).current <= 0.0f) {
            continue;
        }
        return candidate;
    }

    return EntityHandle{};
}

[[nodiscard]] bool
target_is_valid(const World& world, EntityHandle self, EntityHandle target) noexcept {
    if (!world.is_alive(target)) {
        return false;
    }
    if (world.health(target).current <= 0.0f) {
        return false;
    }
    if (!is_hostile(world.kind(self), world.kind(target))) {
        return false;
    }
    return true;
}

void face_toward(Transform& self, const Transform& target_pos) noexcept {
    const SimFloat dx = target_pos.x - self.x;
    const SimFloat dy = target_pos.y - self.y;
    if ((dx * dx) + (dy * dy) <= SimFloat{1e-6f}) {
        return;
    }
    self.facing_radians = std::atan2(dy, dx);
}

void step_toward(Transform& self, const Transform& target_pos, SimFloat max_step) noexcept {
    const SimFloat dx = target_pos.x - self.x;
    const SimFloat dy = target_pos.y - self.y;
    const SimFloat dist_sq = (dx * dx) + (dy * dy);

    if (dist_sq <= SimFloat{1e-6f}) {
        return;
    }

    self.facing_radians = std::atan2(dy, dx);

    const SimFloat dist = std::sqrt(dist_sq);
    if (dist <= max_step) {
        self.x = target_pos.x;
        self.y = target_pos.y;
    } else {
        const SimFloat inv = max_step / dist;
        self.x += dx * inv;
        self.y += dy * inv;
    }
}

}  // namespace

void update_ai(World& world,
               const SpatialGrid& grid,
               SimCommands& commands,
               std::uint64_t current_tick) {
    const uint32_t capacity = world.capacity();

    for (uint32_t i = 0; i < capacity; ++i) {
        if (!world.is_slot_alive(i)) {
            continue;
        }

        const EntityHandle self = world.handle_at(i);
        AIBehaviour& ai = world.ai_behaviour(self);

        if (ai.state == AIState::Inactive) {
            continue;
        }

        if (!ai.target.is_null() && !target_is_valid(world, self, ai.target)) {
            ai.target = EntityHandle{};
            ai.state = AIState::Idle;
        }

        if (ai.state == AIState::Idle) {
            if (current_tick < ai.next_scan_tick) {
                continue;
            }
            ai.next_scan_tick = current_tick + KIdleRescanCooldownTicks;

            const Transform& self_pos = world.transform(self);
            const EntityHandle found =
                find_nearest_hostile(world, grid, self, self_pos, ai.detection_range);
            if (found.is_null()) {
                continue;
            }
            ai.target = found;
            ai.state = AIState::Chase;
        }

        const Transform& self_pos = world.transform(self);
        const Transform& target_pos = world.transform(ai.target);
        const SimFloat d2 = distance_squared(self_pos, target_pos);

        if (ai.leash_range > 0.0f && d2 > ai.leash_range * ai.leash_range) {
            ai.target = EntityHandle{};
            ai.state = AIState::Idle;
            continue;
        }

        const SimFloat attack_range_sq = ai.attack_range * ai.attack_range;
        ai.state = (d2 <= attack_range_sq) ? AIState::Attack : AIState::Chase;

        if (ai.state == AIState::Chase) {
            Transform& mutable_self_pos = world.transform(self);
            step_toward(mutable_self_pos, target_pos, ai.move_speed);
            continue;
        }

        {
            Transform& mutable_self_pos = world.transform(self);
            face_toward(mutable_self_pos, target_pos);
        }

        if (current_tick < ai.next_cast_attempt_tick) {
            continue;
        }
        ai.next_cast_attempt_tick = current_tick + KCastAttemptCooldownTicks;

        commands.cast_ability.push_back(CastAbilityCommand{
            .ability_id = ai.preferred_ability,
            .caster = self,
            .target = ai.target,
            .target_x = target_pos.x,
            .target_y = target_pos.y,
        });
    }
}

}  // namespace sim