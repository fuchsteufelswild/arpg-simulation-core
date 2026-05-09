#pragma once

#include "sim/effect.hpp"
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

struct CastAbilityCommand {
    AbilityId ability_id = NoAbility;
    EntityHandle caster;
    EntityHandle target;
    SimFloat target_x = 0.0f;
    SimFloat target_y = 0.0f;
};

struct SimCommands {
    std::vector<DealDamageCommand> deal_damage;
    std::vector<KillEntityCommand> kill_entity;
    std::vector<CastAbilityCommand> cast_ability;

    void clear() noexcept;
    [[nodiscard]] bool empty() const noexcept;
};

}  // namespace sim