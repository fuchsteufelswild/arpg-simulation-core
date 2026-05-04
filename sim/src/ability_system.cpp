#include "sim/ability_system.hpp"

namespace sim {

void AbilitySystem::add(AbilityInstance instance) {
    switch (instance.phase) {
    case AbilityPhase::Casting:
        casting_.push_back(std::move(instance));
        break;
    case AbilityPhase::Traveling:
        traveling_.push_back(std::move(instance));
        break;
    case AbilityPhase::Resolving:
        resolving_.push_back(std::move(instance));
        break;
    case AbilityPhase::Finished:
        break;
    }
}

void AbilitySystem::clear() noexcept {
    casting_.clear();
    traveling_.clear();
    resolving_.clear();
}

}  // namespace sim