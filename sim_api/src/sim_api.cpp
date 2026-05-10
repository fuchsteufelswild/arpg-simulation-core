#include "sim_api/sim_api.h"

#include "sim/input_command.hpp"
#include "sim/sim.hpp"
#include "sim/version.hpp"

#include <cstdint>
#include <new>

#include "sim_opaque.hpp"

namespace {

[[nodiscard]] sim_api::SimOpaque* unwrap(SimHandle handle) noexcept {
    return reinterpret_cast<sim_api::SimOpaque*>(handle);
}

[[nodiscard]] SimHandle wrap(sim_api::SimOpaque* sim) noexcept {
    return reinterpret_cast<SimHandle>(sim);
}

void rebuild_snapshot(sim_api::SimOpaque& opaque) {
    auto& buffer = opaque.snapshot_buffer;
    buffer.clear();

    const sim::World& world = opaque.sim.world();
    const uint32_t capacity = world.capacity();
    buffer.reserve(world.alive_count());

    std::unordered_map<uint32_t, std::pair<uint16_t, float>> casting_by_caster;

    for (const sim::AbilityInstance& inst : opaque.sim.abilities().casting()) {
        const uint64_t total =
            (inst.resolve_tick > inst.start_tick) ? (inst.resolve_tick - inst.start_tick) : 1;
        const uint64_t elapsed = (opaque.sim.current_tick() > inst.start_tick)
                                     ? (opaque.sim.current_tick() - inst.start_tick)
                                     : 0;
        const float progress = static_cast<float>(elapsed) / static_cast<float>(total);

        casting_by_caster[inst.caster.index] =
            std::make_pair(static_cast<uint16_t>(inst.definition_id), progress);
    }

    for (uint32_t i = 0; i < capacity; ++i) {
        if (!world.is_slot_alive(i)) {
            continue;
        }
        const sim::EntityHandle handle = world.handle_at(i);
        const sim::Transform& transform = world.transform(handle);
        const sim::Health& health = world.health(handle);
        const std::pair<uint16_t, float> casting_info =
            casting_by_caster.contains(handle.index)
                ? casting_by_caster[handle.index]
                : std::make_pair(static_cast<uint16_t>(sim::NoAbility), 0.0f);

        EntitySnapshot snap{};
        snap.handle_index = handle.index;
        snap.handle_generation = handle.generation;
        snap.pos_x = transform.x;
        snap.pos_y = transform.y;
        snap.facing_radians = transform.facing_radians;
        snap.health_current = health.current;
        snap.health_max = health.max;
        snap.status_tags = world.status_list(handle).combined_tags();
        snap.entity_kind = static_cast<uint16_t>(world.kind(handle));
        snap.casting_ability_id = casting_info.first;
        snap.casting_progress = casting_info.second;
        snap.pad0 = 0;
        buffer.push_back(snap);
    }

    opaque.projectile_buffer.clear();
    for (const sim::AbilityInstance& inst : opaque.sim.abilities().traveling()) {
        ProjectileSnapshot proj{};
        proj.caster_index = inst.caster.index;
        proj.caster_generation = inst.caster.generation;
        proj.current_x = inst.current_x;
        proj.current_y = inst.current_y;
        proj.target_x = inst.target_x;
        proj.target_y = inst.target_y;
        proj.ability_id = static_cast<uint16_t>(inst.definition_id);
        opaque.projectile_buffer.push_back(proj);
    }

    opaque.damage_event_buffer.clear();
    for (const sim::DamageEvent& e : opaque.sim.damage_events()) {
        DamageEventSnapshot snap{};
        snap.target_index = e.target.index;
        snap.target_generation = e.target.generation;
        snap.attacker_index = e.attacker.index;
        snap.attacker_generation = e.attacker.generation;
        snap.amount = e.amount;
        snap.ability_tags = e.ability_tags;
        snap.was_crit = e.was_crit ? 1 : 0;
        opaque.damage_event_buffer.push_back(snap);
    }
}

[[nodiscard]] sim::InputCommand convert(const InputCmd& c) noexcept {
    switch (c.type) {
    case SIM_CMD_CAST_ABILITY:
        return sim::InputCastCommand{
            .caster =
                sim::EntityHandle{
                    .index = c.payload.cast.caster_index,
                    .generation = static_cast<uint16_t>(c.payload.cast.caster_generation),
                },
            .ability_id = c.payload.cast.ability_id,
            .target =
                sim::EntityHandle{
                    .index = c.payload.cast.target_index,
                    .generation = static_cast<uint16_t>(c.payload.cast.target_generation),
                },
            .target_x = c.payload.cast.target_x,
            .target_y = c.payload.cast.target_y,
        };
    case SIM_CMD_MOVE_INTENT:
        return sim::InputMoveCommand{
            .entity =
                sim::EntityHandle{
                    .index = c.payload.move.entity_index,
                    .generation = static_cast<uint16_t>(c.payload.move.entity_generation),
                },
            .direction_x = c.payload.move.direction_x,
            .direction_y = c.payload.move.direction_y,
        };
    default:
        return sim::InputMoveCommand{};
    }
}

}  // namespace

extern "C" SIM_API const char* sim_version(void) {
    return sim::version().data();
}

extern "C" SIM_API SimHandle sim_create(uint32_t seed) {
    try {
        auto* opaque = new (std::nothrow) sim_api::SimOpaque(seed);
        return wrap(opaque);
    } catch (...) {
        return nullptr;
    }
}

extern "C" SIM_API void sim_destroy(SimHandle sim) {
    delete unwrap(sim);
}

extern "C" SIM_API void sim_advance(SimHandle sim, uint64_t target_tick) {
    auto* opaque = unwrap(sim);
    if (opaque == nullptr) {
        return;
    }
    try {
        opaque->sim.advance_to(target_tick);
    } catch (const std::exception& e) {
#ifdef SIM_DEBUG
        if (opaque != nullptr) {
            opaque->error_log.emplace_back(e.what());
        }
#endif
    } catch (...) {
#ifdef SIM_DEBUG
        if (opaque != nullptr) {
            opaque->error_log.emplace_back("unknown exception");
        }
#endif
    }
}

extern "C" SIM_API void sim_submit_commands(SimHandle sim, const InputCmdBuffer* commands) {
    auto* opaque = unwrap(sim);
    if (opaque == nullptr || commands == nullptr || commands->commands == nullptr) {
        return;
    }
    try {
        for (uint32_t i = 0; i < commands->count; ++i) {
            opaque->sim.submit_input(convert(commands->commands[i]));
        }
    } catch (const std::exception& e) {
#ifdef SIM_DEBUG
        if (opaque != nullptr) {
            opaque->error_log.emplace_back(e.what());
        }
#endif
    } catch (...) {
#ifdef SIM_DEBUG
        if (opaque != nullptr) {
            opaque->error_log.emplace_back("unknown exception");
        }
#endif
    }
}

extern "C" SIM_API void sim_get_snapshot(SimHandle sim, Snapshot* out_snapshot) {
    if (out_snapshot == nullptr) {
        return;
    }
    auto* opaque = unwrap(sim);
    if (opaque == nullptr) {
        out_snapshot->tick = 0;
        out_snapshot->entity_count = 0;
        out_snapshot->pad0 = 0;
        out_snapshot->entities = nullptr;
        out_snapshot->projectiles = nullptr;
        out_snapshot->damage_events = nullptr;
        return;
    }

    try {
        rebuild_snapshot(*opaque);
    } catch (...) {
    }

    out_snapshot->tick = opaque->sim.current_tick();
    out_snapshot->entity_count = static_cast<uint32_t>(opaque->snapshot_buffer.size());
    out_snapshot->pad0 = 0;
    out_snapshot->entities = opaque->snapshot_buffer.data();
    out_snapshot->projectiles = opaque->projectile_buffer.data();
    out_snapshot->damage_events = opaque->damage_event_buffer.data();
    out_snapshot->projectile_count = static_cast<uint32_t>(opaque->projectile_buffer.size());
    out_snapshot->damage_event_count = static_cast<uint32_t>(opaque->damage_event_buffer.size());
}