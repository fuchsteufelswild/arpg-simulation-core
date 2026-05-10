#include "sim/data/enum_lookup.hpp"

#include <array>
#include <cctype>
#include <string_view>

namespace sim::data {

namespace {

template <typename T>
struct Mapping {
    std::string_view name;
    T value;
};

template <typename T, std::size_t N>
[[nodiscard]] std::optional<T> lookup(const std::array<Mapping<T>, N>& table,
                                      std::string_view name) noexcept {
    for (const auto& entry : table) {
        if (entry.name == name) {
            return entry.value;
        }
    }
    return std::nullopt;
}

constexpr std::array<Mapping<StatId>, 16> StatTable{{
    {.name = "MaxLife", .value = StatId::MaxLife},
    {.name = "LifeRegen", .value = StatId::LifeRegen},
    {.name = "PhysicalDamage", .value = StatId::PhysicalDamage},
    {.name = "FireDamage", .value = StatId::FireDamage},
    {.name = "ColdDamage", .value = StatId::ColdDamage},
    {.name = "LightningDamage", .value = StatId::LightningDamage},
    {.name = "ChaosDamage", .value = StatId::ChaosDamage},
    {.name = "AttackSpeed", .value = StatId::AttackSpeed},
    {.name = "CastSpeed", .value = StatId::CastSpeed},
    {.name = "MovementSpeed", .value = StatId::MovementSpeed},
    {.name = "CritChance", .value = StatId::CritChance},
    {.name = "CritMultiplier", .value = StatId::CritMultiplier},
    {.name = "FireResistance", .value = StatId::FireResistance},
    {.name = "ColdResistance", .value = StatId::ColdResistance},
    {.name = "LightningResistance", .value = StatId::LightningResistance},
    {.name = "ChaosResistance", .value = StatId::ChaosResistance},
}};

constexpr std::array<Mapping<ModOp>, 3> ModOpTable{{
    {.name = "Flat", .value = ModOp::Flat},
    {.name = "Increased", .value = ModOp::Increased},
    {.name = "More", .value = ModOp::More},
}};

constexpr std::array<Mapping<ModPhase>, 2> ModPhaseTable{{
    {.name = "Primary", .value = ModPhase::Primary},
    {.name = "Meta", .value = ModPhase::Meta},
}};

constexpr std::array<Mapping<ConditionId>, 6> ConditionTable{{
    {.name = "WhileAtFullLife", .value = ConditionId::WhileAtFullLife},
    {.name = "WhileAtLowLife", .value = ConditionId::WhileAtLowLife},
    {.name = "AgainstChilled", .value = ConditionId::AgainstChilled},
    {.name = "AgainstIgnited", .value = ConditionId::AgainstIgnited},
    {.name = "OnCrit", .value = ConditionId::OnCrit},
    {.name = "CritRecently", .value = ConditionId::CritRecently},
}};

constexpr std::array<Mapping<StatusType>, 4> StatusTable{{
    {.name = "Chill", .value = StatusType::Chill},
    {.name = "Ignite", .value = StatusType::Ignite},
    {.name = "Poison", .value = StatusType::Poison},
    {.name = "Stun", .value = StatusType::Stun},
}};

constexpr std::array<Mapping<TagMask>, 14> TagTable{{
    {.name = "Physical", .value = tags::Physical},
    {.name = "Fire", .value = tags::Fire},
    {.name = "Cold", .value = tags::Cold},
    {.name = "Lightning", .value = tags::Lightning},
    {.name = "Chaos", .value = tags::Chaos},
    {.name = "Spell", .value = tags::Spell},
    {.name = "Attack", .value = tags::Attack},
    {.name = "Projectile", .value = tags::Projectile},
    {.name = "Melee", .value = tags::Melee},
    {.name = "AoE", .value = tags::AoE},
    {.name = "Channeled", .value = tags::Channeled},
    {.name = "Hit", .value = tags::Hit},
    {.name = "DoT", .value = tags::DoT},
    {.name = "Crit", .value = tags::Crit},
}};

}  // namespace

std::optional<StatId> stat_from_string(std::string_view name) noexcept {
    return lookup(StatTable, name);
}

std::optional<ModOp> mod_op_from_string(std::string_view name) noexcept {
    return lookup(ModOpTable, name);
}

std::optional<ModPhase> mod_phase_from_string(std::string_view name) noexcept {
    return lookup(ModPhaseTable, name);
}

std::optional<ConditionId> condition_from_string(std::string_view name) noexcept {
    return lookup(ConditionTable, name);
}

std::optional<StatusType> status_from_string(std::string_view name) noexcept {
    return lookup(StatusTable, name);
}

std::optional<TagMask> tag_from_string(std::string_view name) noexcept {
    return lookup(TagTable, name);
}

TagMask parse_tag_list(std::string_view tags_csv) {
    TagMask result = tags::None;
    std::size_t start = 0;

    while (start <= tags_csv.size()) {
        const std::size_t pipe = tags_csv.find('|', start);
        const std::size_t end = (pipe == std::string_view::npos) ? tags_csv.size() : pipe;

        std::size_t a = start;
        std::size_t b = end;
        while (a < b && (std::isspace(static_cast<unsigned char>(tags_csv[a])) != 0)) {
            ++a;
        }
        while (b > a && (std::isspace(static_cast<unsigned char>(tags_csv[b - 1])) != 0)) {
            --b;
        }

        if (a < b) {
            const std::string_view token = tags_csv.substr(a, b - a);
            if (auto tag = tag_from_string(token)) {
                result |= *tag;
            }
        }

        if (pipe == std::string_view::npos) {
            break;
        }
        start = pipe + 1;
    }

    return result;
}

}  // namespace sim::data