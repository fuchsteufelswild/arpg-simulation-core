#include "sim/damage.hpp"

#include "sim/sim_commands.hpp"
#include "sim/world.hpp"

namespace sim {

void apply_damage(const DealDamageCommand& cmd,
                  World& world,
                  SimCommands& commands,
                  std::vector<DamageEvent>* events_out) {
    if (!world.is_alive(cmd.target)) {
        return;
    }

    if (events_out != nullptr) {
        events_out->push_back(DamageEvent{
            .target = cmd.target,
            .attacker = cmd.attacker,
            .amount = cmd.amount,
            .ability_tags = cmd.ability_tags,
            .was_crit = false,
        });
    }

    Health& health = world.health(cmd.target);
    health.current -= cmd.amount;

    if (health.current <= 0.0f) {
        health.current = 0.0f;
        commands.kill_entity.push_back(KillEntityCommand{.entity = cmd.target});
    }
}

}  // namespace sim