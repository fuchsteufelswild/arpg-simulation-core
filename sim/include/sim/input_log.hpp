#pragma once

#include "sim/input_command.hpp"

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace sim {

struct LoggedInput {
    uint64_t submitted_at_tick = 0;
    InputCommand command;
};

class InputLog {
public:
    void record(uint64_t tick, InputCommand command);

    [[nodiscard]] std::span<const LoggedInput> entries() const noexcept { return entries_; }

    [[nodiscard]] std::size_t size() const noexcept { return entries_.size(); }

    void clear() noexcept;

    [[nodiscard]] std::vector<uint8_t> serialize() const;

    [[nodiscard]] static InputLog deserialize(std::span<const uint8_t> bytes);

    [[nodiscard]] bool save_to_file(std::string_view path) const;

    [[nodiscard]] static InputLog load_from_file(std::string_view path);

private:
    std::vector<LoggedInput> entries_;
};

}  // namespace sim