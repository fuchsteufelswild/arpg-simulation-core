#pragma once

#include "sim/ability_instance.hpp"
#include "sim/entity_handle.hpp"
#include "sim/spatial_grid.hpp"

namespace sim {

struct AbilityDefinition;
class World;
class EntityStats;
struct SimCommands;
class SpatialGrid;

struct EffectContext {
    const AbilityDefinition* definition = nullptr;
    const AbilityInstance* instance = nullptr;
    EntityHandle current_target;
    World* world = nullptr;
    const EntityStats* caster_stats = nullptr;
    const EntityStats* target_stats = nullptr;
    SimCommands* commands = nullptr;
    uint64_t current_tick = 0;
    const SpatialGrid* grid = nullptr;
};

}  // namespace sim