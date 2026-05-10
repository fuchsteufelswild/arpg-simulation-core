#include "sim/world.hpp"

#include <cassert>

namespace sim {

EntityHandle World::spawn(EntityKind kind) noexcept {
    uint32_t index{};

    if (!free_indices_.empty()) {
        index = free_indices_.back();
        free_indices_.pop_back();

        kinds_[index] = kind;
        transforms_[index] = {};
        healths_[index] = {};
        stats_[index].clear();
        recent_events_[index].clear();
        status_lists_[index].clear();
        cooldown_lists_[index].clear();
    } else {
        index = static_cast<uint32_t>(generations_.size());
        generations_.push_back(1);
        kinds_.push_back(kind);
        transforms_.emplace_back();
        healths_.emplace_back();
        stats_.emplace_back();
        recent_events_.emplace_back();
        status_lists_.emplace_back();
        cooldown_lists_.emplace_back();
    }

    ++alive_count_;
    return EntityHandle{
        .index = index,
        .generation = generations_[index],
    };
}

void World::kill(EntityHandle handle) noexcept {
    if (!is_alive(handle)) {
        return;
    }

    const uint32_t index = handle.index;

    ++generations_[index];
    if (generations_[index] == 0) {
        generations_[index] = 1;
    }

    kinds_[index] = EntityKind::None;

    free_indices_.push_back(index);
    --alive_count_;
}

bool World::is_alive(EntityHandle handle) const noexcept {
    if (handle.is_null()) {
        return false;
    }
    if (!index_in_bounds(handle.index)) {
        return false;
    }
    return generations_[handle.index] == handle.generation;
}

EntityHandle World::handle_at(uint32_t index) const noexcept {
    if (index >= generations_.size()) {
        return {};
    }
    return EntityHandle{.index = index, .generation = generations_[index]};
}

bool World::is_slot_alive(uint32_t index) const noexcept {
    if (index >= generations_.size()) {
        return false;
    }
    return std::ranges::find(free_indices_, index) == free_indices_.end();
}

Transform& World::transform(EntityHandle handle) noexcept {
    assert(is_alive(handle) && "transform() called with stale handle");
    return transforms_[handle.index];
}

const Transform& World::transform(EntityHandle handle) const noexcept {
    assert(is_alive(handle) && "transform() called with stale handle");
    return transforms_[handle.index];
}

Health& World::health(EntityHandle handle) noexcept {
    assert(is_alive(handle) && "health() called with stale handle");
    return healths_[handle.index];
}

const Health& World::health(EntityHandle handle) const noexcept {
    assert(is_alive(handle) && "health() called with stale handle");
    return healths_[handle.index];
}

EntityKind World::kind(EntityHandle handle) const noexcept {
    assert(is_alive(handle) && "kind() called with stale handle");
    return kinds_[handle.index];
}

EntityStats& World::stats(EntityHandle handle) noexcept {
    assert(is_alive(handle) && "stats() called with stale handle");
    return stats_[handle.index];
}

const EntityStats& World::stats(EntityHandle handle) const noexcept {
    assert(is_alive(handle) && "stats() called with stale handle");
    return stats_[handle.index];
}

RecentEvents& World::recent_events(EntityHandle handle) noexcept {
    assert(is_alive(handle) && "recent_events() called with stale handle");
    return recent_events_[handle.index];
}

const RecentEvents& World::recent_events(EntityHandle handle) const noexcept {
    assert(is_alive(handle) && "recent_events() called with stale handle");
    return recent_events_[handle.index];
}

StatusList& World::status_list(EntityHandle handle) noexcept {
    assert(is_alive(handle) && "status_list() called with stale handle");
    return status_lists_[handle.index];
}

const StatusList& World::status_list(EntityHandle handle) const noexcept {
    assert(is_alive(handle) && "status_list() called with stale handle");
    return status_lists_[handle.index];
}

CooldownList& World::cooldowns(EntityHandle handle) noexcept {
    assert(is_alive(handle) && "cooldowns() called with stale handle");
    return cooldown_lists_[handle.index];
}

const CooldownList& World::cooldowns(EntityHandle handle) const noexcept {
    assert(is_alive(handle) && "cooldowns() called with stale handle");
    return cooldown_lists_[handle.index];
}

}  // namespace sim