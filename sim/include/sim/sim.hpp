#pragma once

#include "sim/ability_registry.hpp"
#include "sim/ability_system.hpp"
#include "sim/damage_event.hpp"
#include "sim/input_command.hpp"
#include "sim/sim_commands.hpp"
#include "sim/sim_rng.hpp"
#include "sim/spatial_grid.hpp"
#include "sim/world.hpp"

#include <cstdint>
#include <vector>

namespace sim {

class Sim {
public:
    explicit Sim(uint64_t seed = 1);

    void submit_input(InputCommand command);
    void submit_inputs(std::span<const InputCommand> commands);

    void tick();
    void advance_to(uint64_t target_tick);

    [[nodiscard]] uint64_t current_tick() const noexcept { return current_tick_; }

    [[nodiscard]] World& world() noexcept { return world_; }
    [[nodiscard]] const World& world() const noexcept { return world_; }

    [[nodiscard]] AbilityRegistry& registry() noexcept { return registry_; }
    [[nodiscard]] const AbilityRegistry& registry() const noexcept { return registry_; }

    [[nodiscard]] const AbilitySystem& abilities() const noexcept { return abilities_; }

    [[nodiscard]] const SpatialGrid& spatial_grid() const noexcept { return grid_; }

    [[nodiscard]] SimRng& rng() noexcept { return rng_; }
    [[nodiscard]] const SimRng& rng() const noexcept { return rng_; }

    [[nodiscard]] std::span<const DamageEvent> damage_events() const noexcept {
        return damage_events_;
    }

private:
    void process_input_commands();
    void update_systems();
    void apply_command_buffer();

    World world_;
    AbilityRegistry registry_;
    AbilitySystem abilities_;
    SpatialGrid grid_;
    SimRng rng_;

    SimCommands commands_;
    std::vector<InputCommand> pending_inputs_;

    std::vector<DamageEvent> damage_events_;

    uint64_t current_tick_ = 0;
};

}  // namespace sim