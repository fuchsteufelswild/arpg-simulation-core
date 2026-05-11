#include "sim/ability_registry.hpp"

namespace sim {

AbilityId AbilityRegistry::register_ability(AbilityDefinition def) {
    const auto new_id = static_cast<AbilityId>(definitions_.size() + 1);
    def.id = new_id;
    definitions_.push_back(std::move(def));
    return new_id;
}

const AbilityDefinition* AbilityRegistry::find(AbilityId id) const noexcept {
    if (id == NoAbility) {
        return nullptr;
    }
    const auto index = static_cast<size_t>(id) - 1;
    if (index >= definitions_.size()) {
        return nullptr;
    }
    return &definitions_[index];
}

AbilityId AbilityRegistry::find_by_name(std::string_view name) const noexcept {
    for (const auto& def : definitions_) {
        if (def.name == name) {
            return def.id;
        }
    }
    return NoAbility;
}

void AbilityRegistry::clear() noexcept {
    definitions_.clear();
}

}  // namespace sim