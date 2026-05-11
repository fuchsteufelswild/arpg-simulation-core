#pragma once

#include "sim/ai_behaviour.hpp"
#include "sim/cooldown.hpp"
#include "sim/entity_handle.hpp"
#include "sim/entity_stats.hpp"
#include "sim/recent_events.hpp"
#include "sim/status_list.hpp"

#include <cstdint>
#include <vector>

namespace sim {

struct Transform {
    float x = 0.0f;
    float y = 0.0f;
    float facing_radians = 0.0f;
};

struct Health {
    float current = 0.0f;
    float max = 0.0f;
};

enum class EntityKind : uint16_t {
    None = 0,
    Player,
    Enemy,
    Projectile,
};

class World {
public:
    [[nodiscard]] EntityHandle spawn(EntityKind kind) noexcept;
    void kill(EntityHandle handle) noexcept;

    [[nodiscard]] bool is_alive(EntityHandle handle) const noexcept;
    [[nodiscard]] bool is_slot_alive(uint32_t index) const noexcept;
    [[nodiscard]] EntityHandle handle_at(uint32_t index) const noexcept;

    [[nodiscard]] uint32_t alive_count() const noexcept { return alive_count_; }
    [[nodiscard]] uint32_t capacity() const noexcept {
        return static_cast<uint32_t>(generations_.size());
    }

    [[nodiscard]] Transform& transform(EntityHandle handle) noexcept;
    [[nodiscard]] const Transform& transform(EntityHandle handle) const noexcept;
    [[nodiscard]] Health& health(EntityHandle handle) noexcept;
    [[nodiscard]] const Health& health(EntityHandle handle) const noexcept;
    [[nodiscard]] EntityKind kind(EntityHandle handle) const noexcept;
    [[nodiscard]] EntityStats& stats(EntityHandle handle) noexcept;
    [[nodiscard]] const EntityStats& stats(EntityHandle handle) const noexcept;
    [[nodiscard]] RecentEvents& recent_events(EntityHandle handle) noexcept;
    [[nodiscard]] const RecentEvents& recent_events(EntityHandle handle) const noexcept;
    [[nodiscard]] StatusList& status_list(EntityHandle handle) noexcept;
    [[nodiscard]] const StatusList& status_list(EntityHandle handle) const noexcept;
    [[nodiscard]] CooldownList& cooldowns(EntityHandle handle) noexcept;
    [[nodiscard]] const CooldownList& cooldowns(EntityHandle handle) const noexcept;
    [[nodiscard]] AIBehaviour& ai_behaviour(EntityHandle handle) noexcept;
    [[nodiscard]] const AIBehaviour& ai_behaviour(EntityHandle handle) const noexcept;

private:
    [[nodiscard]] bool index_in_bounds(uint32_t index) const noexcept {
        return index < generations_.size();
    }

    std::vector<uint16_t> generations_;
    std::vector<EntityKind> kinds_;
    std::vector<Transform> transforms_;
    std::vector<Health> healths_;
    std::vector<RecentEvents> recent_events_;
    std::vector<EntityStats> stats_;
    std::vector<StatusList> status_lists_;
    std::vector<CooldownList> cooldown_lists_;
    std::vector<AIBehaviour> ai_behaviours_;

    std::vector<uint32_t> free_indices_;

    uint32_t alive_count_ = 0;
};

}  // namespace sim