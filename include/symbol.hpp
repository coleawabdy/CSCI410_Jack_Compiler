#pragma once

#include <fmt/format.h>

#include <cstdint>

struct symbol {
    enum struct segment_t {
        LOCAL,
        ARGUMENT,
        STATIC,
        THIS
    };

    static std::string segment_to_string(segment_t segment) {
        switch(segment) {
            case segment_t::LOCAL:
                return "local";
            case segment_t::ARGUMENT:
                return "argument";
            case segment_t::STATIC:
                return "static";
            case segment_t::THIS:
                return "this";
        }
    }

    segment_t segment;
    uint16_t index;

    symbol(segment_t segment, uint16_t index) : segment(segment), index(index) {};
    symbol() = default;

    [[nodiscard]] std::string to_string() const { return fmt::format("{} {}", segment_to_string(segment), index); };
};