#pragma once
#include <string_view>

struct Command {
    int operation;
    int first;
    int second;
};

/// Parse exactly three ASCII integers, optionally followed by one legacy NUL.
Command parse_command(std::string_view packet);
