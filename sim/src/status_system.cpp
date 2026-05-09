#include "sim/status_system.hpp"

#include "sim/sim_commands.hpp"
#include "sim/status_list.hpp"
#include "sim/util/overload.hpp"
#include "sim/world.hpp"

#include <variant>

namespace sim {

namespace {

inline constexpr uint64_t DotTickInterval = 30;

void process_dot_tick(StatusInstance& status,
                      EntityHandle target,
                      SimCommands& commands,
                      uint64_t current_tick) {
    if (current_tick < status.last_tick_processed + DotTickInterval) {
        return;
    }

    std::visit(util::Overload{
                   [&](const IgniteData& d) {
                       commands.deal_damage.push_back(DealDamageCommand{
                           .attacker = status.applier,
                           .target = target,
                           .amount = d.damage_per_tick,
                           .ability_tags = tags::Fire | tags::DoT,
                       });
                   },
                   [&](const PoisonData& d) {
                       commands.deal_damage.push_back(DealDamageCommand{
                           .attacker = status.applier,
                           .target = target,
                           .amount = d.damage_per_tick,
                           .ability_tags = tags::Chaos | tags::DoT,
                       });
                   },
                   [](const ChillData&) {},
                   [](const StunData&) {},
               },
               status.payload);

    status.last_tick_processed = current_tick;
}

}  // namespace

void update_statuses(World& world, SimCommands& commands, uint64_t current_tick) {
    for (uint32_t i = 0; i < world.capacity(); ++i) {
        const EntityHandle handle = world.handle_at(i);
        if (!world.is_slot_alive(i)) {
            continue;
        }

        StatusList& list = world.status_list(handle);

        for (StatusInstance& status : list.all_mut()) {
            process_dot_tick(status, handle, commands, current_tick);
        }

        list.remove_expired(current_tick);
    }
}

}  // namespace sim