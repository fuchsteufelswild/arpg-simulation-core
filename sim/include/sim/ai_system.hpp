#pragma once

#include <cstdint>

namespace sim {

class World;
class SpatialGrid;
struct SimCommands;

void update_ai(World& world,
               const SpatialGrid& grid,
               SimCommands& commands,
               std::uint64_t current_tick);

}  // namespace sim