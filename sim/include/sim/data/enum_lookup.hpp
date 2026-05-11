#pragma once

#include "sim/modifier_kinds.hpp"
#include "sim/stat_id.hpp"
#include "sim/status_type.hpp"
#include "sim/tags.hpp"
#include "sim/world.hpp"

#include <optional>
#include <string_view>

namespace sim::data {

[[nodiscard]] std::optional<StatId> stat_from_string(std::string_view name) noexcept;
[[nodiscard]] std::optional<ModOp> mod_op_from_string(std::string_view name) noexcept;
[[nodiscard]] std::optional<ModPhase> mod_phase_from_string(std::string_view name) noexcept;
[[nodiscard]] std::optional<ConditionId> condition_from_string(std::string_view name) noexcept;
[[nodiscard]] std::optional<StatusType> status_from_string(std::string_view name) noexcept;
[[nodiscard]] std::optional<TagMask> tag_from_string(std::string_view name) noexcept;
[[nodiscard]] std::optional<EntityKind> kind_from_string(std::string_view name) noexcept;

[[nodiscard]] TagMask parse_tag_list(std::string_view tags_csv);

}  // namespace sim::data