#include "sim/command_application.hpp"

#include "sim/ability_system.hpp"
#include "sim/damage.hpp"
#include "sim/resolution_context.hpp"
#include "sim/sim_commands.hpp"
#include "sim/world.hpp"

namespace sim {

void apply_commands(AbilitySystem& abilities,
                    const ResolutionContext& res,
                    std::vector<DamageEvent>* events_out) {
    if (res.world == nullptr || res.commands == nullptr) {
        return;
    }

    constexpr int max_passes = 8;

    for (int pass = 0; pass < max_passes; ++pass) {
        if (res.commands->empty()) {
            break;
        }

        SimCommands snapshot = std::move(*res.commands);
        *res.commands = SimCommands{};

        for (const DealDamageCommand& cmd : snapshot.deal_damage) {
            apply_damage(cmd, *res.world, *res.commands, events_out);
        }

        for (const KillEntityCommand& cmd : snapshot.kill_entity) {
            if (res.world->is_alive(cmd.entity)) {
                res.world->kill(cmd.entity);
            }
        }

        for (const CastAbilityCommand& cmd : snapshot.cast_ability) {
            abilities.cast(cmd.ability_id, cmd.caster, cmd.target, cmd.target_x, cmd.target_y, res);
        }
    }
}

}  // namespace sim