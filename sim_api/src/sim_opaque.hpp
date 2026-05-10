#pragma once

#include "sim_api/sim_api.h"

#include "sim/sim.hpp"

#include <string>
#include <vector>

namespace sim_api {

struct SimOpaque {
    sim::Sim sim;
    std::vector<EntitySnapshot> snapshot_buffer;
    std::vector<ProjectileSnapshot> projectile_buffer;
    std::vector<DamageEventSnapshot> damage_event_buffer;
    std::vector<CooldownEntry> cooldown_buffer;

#ifdef SIM_DEBUG
    std::vector<std::string> error_log;
#endif

    explicit SimOpaque(uint32_t seed) : sim(seed) {}
};

}  // namespace sim_api