#include "sim/input_log.hpp"

#include "sim/util/overload.hpp"

#include <cstring>
#include <fstream>
#include <variant>

namespace sim {

namespace {

constexpr uint32_t LogMagic = 0x504C4753;
constexpr uint32_t LogVersion = 1;

constexpr uint8_t TagCast = 1;
constexpr uint8_t TagMove = 2;

void append_u32(std::vector<uint8_t>& out, uint32_t value) {
    out.push_back(static_cast<uint8_t>(value & 0xFFu));
    out.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
    out.push_back(static_cast<uint8_t>((value >> 16) & 0xFFu));
    out.push_back(static_cast<uint8_t>((value >> 24) & 0xFFu));
}

void append_u64(std::vector<uint8_t>& out, uint64_t value) {
    append_u32(out, static_cast<uint32_t>(value & 0xFFFFFFFFu));
    append_u32(out, static_cast<uint32_t>(value >> 32));
}

void append_u16(std::vector<uint8_t>& out, uint16_t value) {
    out.push_back(static_cast<uint8_t>(value & 0xFFu));
    out.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
}

void append_float(std::vector<uint8_t>& out, float value) {
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    append_u32(out, bits);
}

[[nodiscard]] uint32_t read_u32(std::span<const uint8_t> bytes, std::size_t& cursor) {
    const uint32_t v = static_cast<uint32_t>(bytes[cursor]) |
                       (static_cast<uint32_t>(bytes[cursor + 1]) << 8) |
                       (static_cast<uint32_t>(bytes[cursor + 2]) << 16) |
                       (static_cast<uint32_t>(bytes[cursor + 3]) << 24);
    cursor += 4;
    return v;
}

[[nodiscard]] uint64_t read_u64(std::span<const uint8_t> bytes, std::size_t& cursor) {
    const uint64_t lo = read_u32(bytes, cursor);
    const uint64_t hi = read_u32(bytes, cursor);
    return lo | (hi << 32);
}

[[nodiscard]] uint16_t read_u16(std::span<const uint8_t> bytes, std::size_t& cursor) {
    const uint16_t v =
        static_cast<uint16_t>(bytes[cursor]) | static_cast<uint16_t>(bytes[cursor + 1] << 8);
    cursor += 2;
    return v;
}

[[nodiscard]] float read_float(std::span<const uint8_t> bytes, std::size_t& cursor) {
    const uint32_t bits = read_u32(bytes, cursor);
    float value = 0.0f;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

}  // namespace

void InputLog::record(uint64_t tick, InputCommand command) {
    entries_.push_back(LoggedInput{
        .submitted_at_tick = tick,
        .command = command,
    });
}

void InputLog::clear() noexcept {
    entries_.clear();
}

std::vector<uint8_t> InputLog::serialize() const {
    std::vector<uint8_t> out;
    out.reserve(16 + (entries_.size() * 32));

    append_u32(out, LogMagic);
    append_u32(out, LogVersion);
    append_u64(out, static_cast<uint64_t>(entries_.size()));

    for (const LoggedInput& entry : entries_) {
        append_u64(out, entry.submitted_at_tick);

        std::visit(util::Overload{
                       [&](const InputCastCommand& c) {
                           out.push_back(TagCast);
                           append_u32(out, c.caster.index);
                           append_u16(out, c.caster.generation);
                           append_u16(out, c.ability_id);
                           append_u32(out, c.target.index);
                           append_u16(out, c.target.generation);
                           append_u16(out, 0);
                           append_float(out, c.target_x);
                           append_float(out, c.target_y);
                       },
                       [&](const InputMoveCommand& m) {
                           out.push_back(TagMove);
                           append_u32(out, m.entity.index);
                           append_u16(out, m.entity.generation);
                           append_u16(out, 0);
                           append_float(out, m.direction_x);
                           append_float(out, m.direction_y);
                           append_u64(out, 0);
                       },
                   },
                   entry.command);
    }

    return out;
}

InputLog InputLog::deserialize(std::span<const uint8_t> bytes) {
    InputLog log;
    if (bytes.size() < 16) {
        return log;
    }

    std::size_t cursor = 0;
    const uint32_t magic = read_u32(bytes, cursor);
    const uint32_t version = read_u32(bytes, cursor);
    if (magic != LogMagic || version != LogVersion) {
        return log;
    }

    const uint64_t count = read_u64(bytes, cursor);
    log.entries_.reserve(count);

    for (uint64_t i = 0; i < count; ++i) {
        if (cursor + 9 > bytes.size()) {
            break;
        }
        const uint64_t tick = read_u64(bytes, cursor);
        const uint8_t tag = bytes[cursor++];

        if (tag == TagCast) {
            const uint32_t caster_index = read_u32(bytes, cursor);
            const uint16_t caster_gen = read_u16(bytes, cursor);
            const uint16_t ability_id = read_u16(bytes, cursor);
            const uint32_t target_index = read_u32(bytes, cursor);
            const uint16_t target_gen = read_u16(bytes, cursor);
            cursor += 2;
            const float tx = read_float(bytes, cursor);
            const float ty = read_float(bytes, cursor);

            log.entries_.push_back(LoggedInput{
                .submitted_at_tick = tick,
                .command =
                    InputCastCommand{
                        .caster = EntityHandle{.index = caster_index, .generation = caster_gen},
                        .ability_id = ability_id,
                        .target = EntityHandle{.index = target_index, .generation = target_gen},
                        .target_x = tx,
                        .target_y = ty,
                    },
            });
        } else if (tag == TagMove) {
            const uint32_t entity_index = read_u32(bytes, cursor);
            const uint16_t entity_gen = read_u16(bytes, cursor);
            cursor += 2;
            const float dx = read_float(bytes, cursor);
            const float dy = read_float(bytes, cursor);
            cursor += 8;

            log.entries_.push_back(LoggedInput{
                .submitted_at_tick = tick,
                .command =
                    InputMoveCommand{
                        .entity = EntityHandle{.index = entity_index, .generation = entity_gen},
                        .direction_x = dx,
                        .direction_y = dy,
                    },
            });
        }
    }

    return log;
}

bool InputLog::save_to_file(std::string_view path) const {
    std::ofstream file{std::string{path}, std::ios::binary};
    if (!file.is_open()) {
        return false;
    }
    const auto bytes = serialize();
    file.write(reinterpret_cast<const char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
    return file.good();
}

InputLog InputLog::load_from_file(std::string_view path) {
    std::ifstream file{std::string{path}, std::ios::binary};
    if (!file.is_open()) {
        return {};
    }
    file.seekg(0, std::ios::end);
    const auto size = file.tellg();
    if (size <= 0) {
        return {};
    }
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> bytes(static_cast<std::size_t>(size));
    file.read(reinterpret_cast<char*>(bytes.data()), size);
    return deserialize(bytes);
}

}  // namespace sim