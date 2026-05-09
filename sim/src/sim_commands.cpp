#include "sim/sim_commands.hpp"

namespace sim {

void SimCommands::clear() noexcept {
    deal_damage.clear();
    kill_entity.clear();
    cast_ability.clear();
}

bool SimCommands::empty() const noexcept {
    return deal_damage.empty() && kill_entity.empty() && cast_ability.empty();
}

}  // namespace sim