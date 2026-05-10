#include "sim/replay.hpp"

#include "sim/sim.hpp"
#include "sim/world_hash.hpp"

namespace sim {

ReplayResult
replay(const InputLog& log, uint64_t seed, uint64_t target_tick, const SimSetupFn& setup) {
    Sim sim(seed);
    if (setup) {
        setup(sim);
    }

    const auto entries = log.entries();
    std::size_t next_input = 0;

    while (sim.current_tick() < target_tick) {
        while (next_input < entries.size() &&
               entries[next_input].submitted_at_tick == sim.current_tick()) {
            sim.submit_input(entries[next_input].command);
            ++next_input;
        }
        sim.tick();
    }

    return ReplayResult{
        .final_hash = hash_world_state(sim),
        .ticks_run = sim.current_tick(),
    };
}

}  // namespace sim