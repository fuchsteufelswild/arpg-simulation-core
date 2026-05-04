#pragma once

#include "sim/ability_instance.hpp"
#include "sim/entity_handle.hpp"
#include "sim/sim_float.hpp"

#include <span>
#include <vector>

namespace sim {

class AbilityRegistry;
class World;
struct SimCommands;

class AbilitySystem {
public:
    void add(AbilityInstance instance);

    void cast(AbilityId ability_id,
              EntityHandle caster,
              EntityHandle target,
              SimFloat target_x,
              SimFloat target_y,
              const AbilityRegistry& registry,
              World& world,
              SimCommands& commands,
              uint64_t current_tick);

    void update(const AbilityRegistry& registry,
                World& world,
                SimCommands& commands,
                uint64_t current_tick);

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