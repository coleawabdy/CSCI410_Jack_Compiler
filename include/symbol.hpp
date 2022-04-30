#pragma once

#include <cstdint>

struct symbol {
    enum struct segment_t {
        LOCAL,
        ARGUMENT,
        STATIC
    };

    segment_t segment;
    uint16_t index;
};