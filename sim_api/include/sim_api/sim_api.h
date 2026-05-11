#pragma once

#include <stdint.h>

#ifdef SIM_API_BUILDING_DLL
#define SIM_API __declspec(dllexport)
#else
#define SIM_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SimOpaque* SimHandle;

typedef struct EntitySnapshot {
    uint32_t handle_index;
    uint32_t handle_generation;
    float pos_x;
    float pos_y;
    float facing_radians;
    float health_current;
    float health_max;
    uint32_t pad0;
    uint64_t status_tags;
    uint16_t entity_kind;
    uint16_t casting_ability_id;
    float casting_progress;
} EntitySnapshot;

typedef struct ProjectileSnapshot {
    uint32_t caster_index;
    uint32_t caster_generation;
    float current_x;
    float current_y;
    float target_x;
    float target_y;
    uint16_t ability_id;
    uint16_t pad0;
    uint32_t pad1;
} ProjectileSnapshot;

typedef struct DamageEventSnapshot {
    uint64_t ability_tags;
    uint32_t target_index;
    uint32_t target_generation;
    uint32_t attacker_index;
    uint32_t attacker_generation;
    float amount;
    uint16_t pad1;
    uint8_t was_crit;
    uint8_t pad0;
} DamageEventSnapshot;

typedef struct Snapshot {
    uint64_t tick;
    const EntitySnapshot* entities;
    const ProjectileSnapshot* projectiles;
    const DamageEventSnapshot* damage_events;
    uint32_t entity_count;
    uint32_t projectile_count;
    uint32_t damage_event_count;
    uint32_t pad0;
} Snapshot;

#define SIM_CMD_CAST_ABILITY 1u
#define SIM_CMD_MOVE_INTENT 2u

typedef struct CastAbilityCmd {
    uint32_t caster_index;
    uint32_t caster_generation;
    uint16_t ability_id;
    uint16_t pad0;
    uint32_t target_index;
    uint32_t target_generation;
    float target_x;
    float target_y;
} CastAbilityCmd;

typedef struct MoveIntentCmd {
    uint32_t entity_index;
    uint32_t entity_generation;
    float direction_x;
    float direction_y;
} MoveIntentCmd;

typedef struct InputCmd {
    uint32_t type;
    uint32_t pad0;
    union {
        CastAbilityCmd cast;
        MoveIntentCmd move;
    } payload;
} InputCmd;

typedef struct InputCmdBuffer {
    uint32_t count;
    uint32_t pad0;
    const InputCmd* commands;
} InputCmdBuffer;

typedef struct CooldownEntry {
    uint16_t ability_id;
    uint16_t pad0;
    uint32_t remaining_ticks;
} CooldownEntry;

typedef struct CooldownSnapshot {
    uint32_t entity_index;
    uint32_t entity_generation;
    uint32_t count;
    uint32_t pad0;
    const CooldownEntry* entries;
} CooldownSnapshot;

SIM_API const char* sim_version(void);

SIM_API SimHandle sim_create(uint32_t seed);

SIM_API int32_t sim_load_content(SimHandle sim,
                                 const char* abilities_toml,
                                 size_t abilities_len,
                                 const char* archetypes_toml,
                                 size_t archetypes_len);

SIM_API void sim_destroy(SimHandle sim);

SIM_API void sim_advance(SimHandle sim, uint64_t target_tick);

SIM_API void sim_submit_commands(SimHandle sim, const InputCmdBuffer* commands);

SIM_API void sim_get_snapshot(SimHandle sim, Snapshot* out_snapshot);

// The returned entries pointer is valid only until the next call to
// sim_get_cooldowns or any other sim_* function. Copy immediately.
SIM_API void sim_get_cooldowns(SimHandle sim,
                               uint32_t entity_index,
                               uint32_t entity_generation,
                               CooldownSnapshot* out_snapshot);

// Debug-only spawn API. Production code should drive spawning from sim-side
// level data, not from the client. Exposed to allow Unity demos and tests
// to populate the world without scene-loading infrastructure.
SIM_API uint64_t sim_debug_spawn_entity(SimHandle sim,
                                        uint16_t entity_kind,
                                        float pos_x,
                                        float pos_y);

SIM_API uint64_t sim_spawn_archetype(SimHandle sim,
                                     const char* archetype_name,
                                     float pos_x,
                                     float pos_y);

#ifdef __cplusplus
}
#endif