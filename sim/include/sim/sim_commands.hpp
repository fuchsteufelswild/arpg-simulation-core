#pragma once

#include "sim/entity_handle.hpp"
#include "sim/sim_float.hpp"
#include "sim/tags.hpp"

#include <vector>

namespace sim {

struct DealDamageCommand {
    EntityHandle attacker;
    EntityHandle target;
    SimFloat amount = 0.0f;
    TagMask ability_tags = tags::None;
};

struct KillEntityCommand {
    EntityHandle entity;
};

struct SimCommands {
    std::vector<DealDamageCommand> deal_damage;
    std::vector<KillEntityCommand> kill_entity;

    void clear() noexcept;
    [[nodiscard]] bool empty() const noexcept;
};

}  // namespace sim