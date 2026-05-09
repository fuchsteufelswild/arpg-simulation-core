#pragma once

#include <cstdint>

namespace sim {

class World;
class AbilityRegistry;
class SpatialGrid;
struct SimCommands;

struct ResolutionContext {
    World* world = nullptr;
    const AbilityRegistry* registry = nullptr;
    const SpatialGrid* grid = nullptr;
    SimCommands* commands = nullptr;
    uint64_t current_tick = 0;
};

}  // namespace sim