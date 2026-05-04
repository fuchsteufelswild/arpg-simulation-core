#include "sim/damage.hpp"

#include "sim/sim_commands.hpp"
#include "sim/world.hpp"

namespace sim {

void apply_damage(const DealDamageCommand& cmd, World& world, SimCommands& commands) {
    if (!world.is_alive(cmd.target)) {
        return;
    }

    Health& health = world.health(cmd.target);
    health.current -= cmd.amount;

    if (health.current <= 0.0f) {
        health.current = 0.0f;
        commands.kill_entity.push_back(KillEntityCommand{.entity = cmd.target});
    }
}

}  // namespace sim