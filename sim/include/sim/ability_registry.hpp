#pragma once

#include "sim/ability_definition.hpp"

#include <vector>

namespace sim {

class AbilityRegistry {
public:
    AbilityId register_ability(AbilityDefinition def);

    [[nodiscard]] const AbilityDefinition* find(AbilityId id) const noexcept;

    [[nodiscard]] AbilityId find_by_name(std::string_view name) const noexcept;

    [[nodiscard]] uint32_t count() const noexcept {
        return static_cast<uint32_t>(definitions_.size());
    }

    void clear() noexcept;

private:
    std::vector<AbilityDefinition> definitions_;
};

}  // namespace sim