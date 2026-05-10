#include "sim/cooldown.hpp"

#include <algorithm>

namespace sim {

void CooldownList::set(AbilityId ability_id, uint64_t ready_at_tick) noexcept {
    auto it = std::ranges::find_if(
        entries_, [ability_id](const CooldownEntry& e) { return e.ability_id == ability_id; });
    if (it != entries_.end()) {
        it->ready_at_tick = ready_at_tick;
        return;
    }
    entries_.push_back(CooldownEntry{
        .ability_id = ability_id,
        .ready_at_tick = ready_at_tick,
    });
}

bool CooldownList::is_ready(AbilityId ability_id, uint64_t current_tick) const noexcept {
    auto it = std::ranges::find_if(
        entries_, [ability_id](const CooldownEntry& e) { return e.ability_id == ability_id; });
    if (it == entries_.end()) {
        return true;
    }
    return current_tick >= it->ready_at_tick;
}

uint64_t CooldownList::ready_at(AbilityId ability_id) const noexcept {
    auto it = std::ranges::find_if(
        entries_, [ability_id](const CooldownEntry& e) { return e.ability_id == ability_id; });
    return it == entries_.end() ? 0 : it->ready_at_tick;
}

void CooldownList::clear() noexcept {
    entries_.clear();
}

}  // namespace sim