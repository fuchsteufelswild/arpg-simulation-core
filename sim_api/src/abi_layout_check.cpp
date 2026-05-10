#include "sim_api/sim_api.h"

#include <type_traits>

static_assert(std::is_standard_layout_v<EntitySnapshot>);
static_assert(std::is_trivially_copyable_v<EntitySnapshot>);
static_assert(sizeof(EntitySnapshot) == 48);
static_assert(alignof(EntitySnapshot) == 8);

static_assert(std::is_standard_layout_v<ProjectileSnapshot>);
static_assert(std::is_trivially_copyable_v<ProjectileSnapshot>);
static_assert(sizeof(ProjectileSnapshot) == 32);
static_assert(alignof(ProjectileSnapshot) == 4);

static_assert(std::is_standard_layout_v<DamageEventSnapshot>);
static_assert(std::is_trivially_copyable_v<DamageEventSnapshot>);
static_assert(sizeof(DamageEventSnapshot) == 32);
static_assert(alignof(DamageEventSnapshot) == 8);

static_assert(std::is_standard_layout_v<EntitySnapshot>);
static_assert(std::is_trivially_copyable_v<EntitySnapshot>);
static_assert(sizeof(EntitySnapshot) == 48);
static_assert(alignof(EntitySnapshot) == 8);

static_assert(std::is_standard_layout_v<Snapshot>);
static_assert(std::is_trivially_copyable_v<Snapshot>);
static_assert(sizeof(Snapshot) == 48);

static_assert(std::is_standard_layout_v<CooldownEntry>);
static_assert(std::is_trivially_copyable_v<CooldownEntry>);
static_assert(sizeof(CooldownEntry) == 8);

static_assert(std::is_standard_layout_v<CooldownSnapshot>);
static_assert(std::is_trivially_copyable_v<CooldownSnapshot>);
static_assert(sizeof(CooldownSnapshot) == 24);

static_assert(std::is_standard_layout_v<CastAbilityCmd>);
static_assert(std::is_trivially_copyable_v<CastAbilityCmd>);
static_assert(sizeof(CastAbilityCmd) == 28);

static_assert(std::is_standard_layout_v<MoveIntentCmd>);
static_assert(std::is_trivially_copyable_v<MoveIntentCmd>);
static_assert(sizeof(MoveIntentCmd) == 16);

static_assert(std::is_standard_layout_v<InputCmd>);
static_assert(std::is_trivially_copyable_v<InputCmd>);