#pragma once
#include <array>
#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>

struct Instruction {
    int type = 0;
    int x = 0;
    int y = 0;
    int code = 0;
    int key_up = 0;
    int cursor = 0; // 1 hides, 2 restores.
    int wheel = 0;
};

inline void validate_instruction(const Instruction& value) {
    if (value.type < 0 || value.type > 3 || value.key_up < 0 || value.key_up > 1) {
        throw std::invalid_argument("Invalid instruction type or key-up flag.");
    }
    if (value.type != 2 && (value.x != 0 || value.y != 0)) {
        throw std::invalid_argument("Only movement instructions use X/Y.");
    }
    if (value.type != 3 && value.cursor != 0) {
        throw std::invalid_argument("Only visibility instructions use the cursor field.");
    }
    if (value.type != 0 && value.wheel != 0) {
        throw std::invalid_argument("Only mouse instructions use the wheel field.");
    }
    switch (value.type) {
    case 0: {
        const bool button = value.code == 2 || value.code == 4 || value.code == 8 ||
                            value.code == 16 || value.code == 32 || value.code == 64;
        const bool wheel = value.code == 0 && value.wheel != 0 && value.key_up == 0;
        if (!(button && value.wheel == 0) && !wheel) {
            throw std::invalid_argument("Expected one mouse button flag or a wheel delta.");
        }
        break;
    }
    case 1:
        if (value.code < 1 || value.code > 254) throw std::invalid_argument("Invalid virtual key.");
        break;
    case 2:
        if (value.code != 0 || value.key_up != 0) throw std::invalid_argument("Movement has unused fields.");
        break;
    case 3:
        if ((value.cursor != 1 && value.cursor != 2) || value.code != 0 || value.key_up != 0) {
            throw std::invalid_argument("Visibility requires cursor=1 or 2 and unused fields zero.");
        }
        break;
    }
}

inline Instruction parse_instruction(std::string_view packet) {
    if (!packet.empty() && packet.back() == '\0') packet.remove_suffix(1);
    if (packet.empty() || packet.size() > 128 || packet.find('\0') != std::string_view::npos) {
        throw std::invalid_argument("Invalid packet length or embedded NUL.");
    }
    const auto whitespace = [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; };
    std::array<int, 7> fields{};
    for (int& field : fields) {
        while (!packet.empty() && whitespace(packet.front())) packet.remove_prefix(1);
        const auto parsed = std::from_chars(packet.data(), packet.data() + packet.size(), field);
        if (parsed.ec != std::errc{} || parsed.ptr == packet.data() ||
            (parsed.ptr != packet.data() + packet.size() && !whitespace(*parsed.ptr))) {
            throw std::invalid_argument("Expected exactly seven whitespace-separated integers.");
        }
        packet.remove_prefix(static_cast<std::size_t>(parsed.ptr - packet.data()));
    }
    while (!packet.empty() && whitespace(packet.front())) packet.remove_prefix(1);
    if (!packet.empty()) throw std::invalid_argument("Trailing packet data.");
    const Instruction value{fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6]};
    validate_instruction(value);
    return value;
}

inline std::string format_instruction(const Instruction& value) {
    validate_instruction(value);
    return std::to_string(value.type) + " " + std::to_string(value.x) + " " +
           std::to_string(value.y) + " " + std::to_string(value.code) + " " +
           std::to_string(value.key_up) + " " + std::to_string(value.cursor) + " " +
           std::to_string(value.wheel);
}
