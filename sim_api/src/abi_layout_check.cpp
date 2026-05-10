#include "sim_api/sim_api.h"

#include <type_traits>

static_assert(std::is_standard_layout_v<EntitySnapshot>);
static_assert(std::is_trivially_copyable_v<EntitySnapshot>);
static_assert(sizeof(EntitySnapshot) == 48);
static_assert(alignof(EntitySnapshot) == 8);

static_assert(std::is_standard_layout_v<Snapshot>);
static_assert(std::is_trivially_copyable_v<Snapshot>);
static_assert(sizeof(Snapshot) == 24);

static_assert(std::is_standard_layout_v<CastAbilityCmd>);
static_assert(std::is_trivially_copyable_v<CastAbilityCmd>);
static_assert(sizeof(CastAbilityCmd) == 28);

static_assert(std::is_standard_layout_v<MoveIntentCmd>);
static_assert(std::is_trivially_copyable_v<MoveIntentCmd>);
static_assert(sizeof(MoveIntentCmd) == 16);

static_assert(std::is_standard_layout_v<InputCmd>);
static_assert(std::is_trivially_copyable_v<InputCmd>);