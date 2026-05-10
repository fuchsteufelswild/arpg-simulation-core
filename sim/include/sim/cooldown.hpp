#pragma once

#include "sim/effect.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace sim {

struct CooldownEntry {
    AbilityId ability_id = NoAbility;
    uint64_t ready_at_tick = 0;
};

class CooldownList {
public:
    void set(AbilityId ability_id, uint64_t ready_at_tick) noexcept;

    [[nodiscard]] bool is_ready(AbilityId ability_id, uint64_t current_tick) const noexcept;

    [[nodiscard]] uint64_t ready_at(AbilityId ability_id) const noexcept;

    [[nodiscard]] std::span<const CooldownEntry> all() const noexcept { return entries_; }

    void clear() noexcept;

    [[nodiscard]] std::size_t size() const noexcept { return entries_.size(); }

private:
    std::vector<CooldownEntry> entries_;
};

}  // namespace sim