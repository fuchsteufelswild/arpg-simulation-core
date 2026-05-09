#pragma once

#include "sim/effect.hpp"
#include "sim/entity_handle.hpp"
#include "sim/sim_float.hpp"

#include <variant>

namespace sim {

struct InputCastCommand {
    EntityHandle caster;
    AbilityId ability_id = NoAbility;
    EntityHandle target;
    SimFloat target_x = 0.0f;
    SimFloat target_y = 0.0f;
};

struct InputMoveCommand {
    EntityHandle entity;
    SimFloat direction_x = 0.0f;
    SimFloat direction_y = 0.0f;
};

using InputCommand = std::variant<InputCastCommand, InputMoveCommand>;

}  // namespace sim