#pragma once

#include <fmt/format.h>

#include <cstdint>
#include <utility>

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

        throw std::runtime_error("failed to convert segment to string");
    }

    segment_t segment = segment_t::LOCAL;
    uint16_t index = 0;
    std::string type;

    symbol(segment_t segment, uint16_t index, std::string type) : segment(segment), index(index), type(std::move(type)) {};
    symbol() = default;

    [[nodiscard]] std::string to_string() const { return fmt::format("{} {}", segment_to_string(segment), index); };
};