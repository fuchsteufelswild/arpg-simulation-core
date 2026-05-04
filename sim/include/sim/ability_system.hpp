#pragma once

#include "sim/ability_instance.hpp"

#include <span>
#include <vector>

namespace sim {

class AbilitySystem {
public:
    void add(AbilityInstance instance);

    [[nodiscard]] std::span<const AbilityInstance> casting() const noexcept { return casting_; }
    [[nodiscard]] std::span<const AbilityInstance> traveling() const noexcept { return traveling_; }
    [[nodiscard]] std::span<const AbilityInstance> resolving() const noexcept { return resolving_; }

    [[nodiscard]] uint32_t total_active() const noexcept {
        return static_cast<uint32_t>(casting_.size() + traveling_.size() + resolving_.size());
    }

    void clear() noexcept;

private:
    std::vector<AbilityInstance> casting_;
    std::vector<AbilityInstance> traveling_;
    std::vector<AbilityInstance> resolving_;
};

}  // namespace sim