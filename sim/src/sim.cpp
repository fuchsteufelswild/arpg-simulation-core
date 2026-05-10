#include "sim/sim.hpp"

#include "sim/command_application.hpp"
#include "sim/resolution_context.hpp"
#include "sim/status_system.hpp"
#include "sim/util/overload.hpp"

#include <variant>

namespace sim {

Sim::Sim(uint64_t seed) : rng_(seed) {
}

void Sim::submit_input(InputCommand command) {
    pending_inputs_.push_back(command);
}

void Sim::submit_inputs(std::span<const InputCommand> commands) {
    pending_inputs_.insert(pending_inputs_.end(), commands.begin(), commands.end());
}

void Sim::tick() {
    damage_events_.clear();
    grid_.rebuild(world_);
    process_input_commands();
    update_systems();
    apply_command_buffer();
    ++current_tick_;
}

void Sim::advance_to(uint64_t target_tick) {
    while (current_tick_ < target_tick) {
        tick();
    }
}

void Sim::process_input_commands() {
    if (pending_inputs_.empty()) {
        return;
    }

    const ResolutionContext ctx{
        .world = &world_,
        .registry = &registry_,
        .grid = &grid_,
        .commands = &commands_,
        .current_tick = current_tick_,
        .rng = &rng_,
    };

    for (const InputCommand& input : pending_inputs_) {
        std::visit(util::Overload{
                       [&](const InputCastCommand& cast) {
                           abilities_.cast(cast.ability_id,
                                           cast.caster,
                                           cast.target,
                                           cast.target_x,
                                           cast.target_y,
                                           ctx);
                       },
                       [&](const InputMoveCommand&) {},
                   },
                   input);
    }

    pending_inputs_.clear();
}

void Sim::update_systems() {
    const ResolutionContext ctx{
        .world = &world_,
        .registry = &registry_,
        .grid = &grid_,
        .commands = &commands_,
        .current_tick = current_tick_,
        .rng = &rng_,
    };

    abilities_.update(ctx);
    update_statuses(world_, commands_, current_tick_);
}

void Sim::apply_command_buffer() {
    if (commands_.empty()) {
        return;
    }

    const ResolutionContext ctx{
        .world = &world_,
        .registry = &registry_,
        .grid = &grid_,
        .commands = &commands_,
        .current_tick = current_tick_,
        .rng = &rng_,
    };

    apply_commands(abilities_, ctx, &damage_events_);
}

}  // namespace sim