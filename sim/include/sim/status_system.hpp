#pragma once

#include <cstdint>

namespace sim {

class World;
struct SimCommands;

void update_statuses(World& world, SimCommands& commands, uint64_t current_tick);

}  // namespace sim