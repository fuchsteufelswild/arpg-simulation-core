#pragma once

#include "sim/sim_rng.hpp"

#include <cstdint>

namespace sim {

class World;
class AbilityRegistry;
class SpatialGrid;
struct SimCommands;
class SimRng;

struct ResolutionContext {
    World* world = nullptr;
    const AbilityRegistry* registry = nullptr;
    const SpatialGrid* grid = nullptr;
    SimCommands* commands = nullptr;
    uint64_t current_tick = 0;
    SimRng* rng = nullptr;
};

}  // namespace sim