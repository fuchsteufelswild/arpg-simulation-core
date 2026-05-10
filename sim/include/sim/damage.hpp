#pragma once

#include "sim/damage_event.hpp"

namespace sim {

class World;
struct DealDamageCommand;
struct SimCommands;

void apply_damage(const DealDamageCommand& cmd,
                  World& world,
                  SimCommands& commands,
                  std::vector<DamageEvent>* events_out);

}  // namespace sim