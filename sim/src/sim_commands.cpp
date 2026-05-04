#include "sim/sim_commands.hpp"

namespace sim {

void SimCommands::clear() noexcept {
    deal_damage.clear();
    kill_entity.clear();
}

bool SimCommands::empty() const noexcept {
    return deal_damage.empty() && kill_entity.empty();
}

}  // namespace sim