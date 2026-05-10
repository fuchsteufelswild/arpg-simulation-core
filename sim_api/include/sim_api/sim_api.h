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
    uint64_t status_tags;
    uint16_t entity_kind;
    uint16_t pad0;
    uint32_t pad1;
} EntitySnapshot;

typedef struct Snapshot {
    uint64_t tick;
    uint32_t entity_count;
    uint32_t pad0;
    const EntitySnapshot* entities;
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

SIM_API const char* sim_version(void);

SIM_API SimHandle sim_create(uint32_t seed);

SIM_API void sim_destroy(SimHandle sim);

SIM_API void sim_advance(SimHandle sim, uint64_t target_tick);

SIM_API void sim_submit_commands(SimHandle sim, const InputCmdBuffer* commands);

SIM_API void sim_get_snapshot(SimHandle sim, Snapshot* out_snapshot);

#ifdef __cplusplus
}
#endif