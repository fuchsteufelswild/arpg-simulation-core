#pragma once

namespace sim {

class World;
struct DealDamageCommand;
struct SimCommands;

void apply_damage(const DealDamageCommand& cmd, World& world, SimCommands& commands);

}  // namespace sim