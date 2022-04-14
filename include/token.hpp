#pragma once

#include <string>
#include <variant>
#include <cstdint>

struct token {
    enum class type_t {
        KEYWORD,
        IDENTIFIER,
        SYMBOL,
        STRING_CONSTANT,
        INT_CONSTANT
    };

    enum class keyword_t {
        CLASS,
        METHOD,
        FUNCTION,
        CONSTRUCTOR,
        INT,
        BOOL,
        CHAR,
        VOID,
        VAR,
        STATIC,
        FIELD,
        LET,
        DO,
        IF,
        ELSE,
        WHILE,
        RETURN,
        TRUE,
        FALSE,
        NUL,
        THIS
    };

    typedef std::string identifier_t;
    typedef uint16_t int_constant_t;
    typedef std::string string_constant_t;
    typedef char symbol_t;

    typedef std::variant<keyword_t, int_constant_t, symbol_t, std::string> value_t;

    type_t type;
    value_t value;

    template <typename Return>
    inline const Return& get_value() const {
        return std::get<Return>(value);
    }
};