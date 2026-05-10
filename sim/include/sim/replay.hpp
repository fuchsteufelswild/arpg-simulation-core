#pragma once

#include "sim/input_log.hpp"

#include <cstdint>
#include <functional>

namespace sim {

class Sim;

struct ReplayResult {
    uint64_t final_hash = 0;
    uint64_t ticks_run = 0;
};

using SimSetupFn = std::function<void(Sim&)>;

[[nodiscard]] ReplayResult
replay(const InputLog& log, uint64_t seed, uint64_t target_tick, const SimSetupFn& setup);

}  // namespace sim