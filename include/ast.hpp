#pragma once

#include "token.hpp"

#include <string>
#include <list>
#include <optional>
#include <utility>

struct ast_parameter {
    std::string type;
    std::string identifier;
};

struct ast_class_variable {
    bool is_static;
    std::string type;
    std::list<std::string> identifiers;
};

struct ast_subroutine_local {
    std::string type;
    std::list<std::string> identifiers;
};

struct ast_term {
    enum struct type_t {
        INTEGER,
        STRING,
        NUL,
        THIS,
        TRUE,
        FALSE,
        VARIABLE,
        ARRAY,
        EXPRESSION,
        UNARY,
        SUBROUTINE_CALL
    };

    type_t type;

    explicit ast_term(type_t type) : type(type) {};
};

enum struct ast_unary_op {
    NEGATE,
    INVERT
};

enum struct ast_binary_op {
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    AND,
    OR,
    GREATER,
    LESSER,
    EQUAL
};

struct ast_expression {
    typedef std::pair<ast_binary_op, ast_term*> op_term_t;

    ast_term* primary = nullptr;
    std::list<op_term_t> secondaries;

    explicit ast_expression(ast_term* primary) : primary(primary) {};
    ast_expression() = default;
};

struct ast_subroutine_call {
    std::optional<std::string> callee_identifier;
    std::string subroutine_identifier;
    std::list<ast_expression> arguments;

    explicit ast_subroutine_call(
            std::string subroutine_identifier = "",
    std::optional<std::string> callee_identifier = std::nullopt)
    : subroutine_identifier(std::move(subroutine_identifier)),
    callee_identifier(std::move(callee_identifier)) {};
};

struct ast_term_integer : public ast_term {
    uint16_t value;
    explicit ast_term_integer(uint16_t value)
    : ast_term(type_t::INTEGER),
    value(value) {};
};

struct ast_term_string : public ast_term {
    std::string value;
    explicit ast_term_string(std::string value)
    : ast_term(type_t::STRING),
    value(std::move(value)) {};
};

struct ast_term_variable : public ast_term {
    std::string identifier;
    explicit ast_term_variable(std::string identifier)
    : ast_term(type_t::VARIABLE),
    identifier(std::move(identifier)) {};
};

struct ast_term_array : public ast_term {
    std::string identifier;
    ast_expression access;
    explicit ast_term_array(ast_expression access)
    : ast_term(type_t::ARRAY),
    access(std::move(access)) {};
};

struct ast_term_expression : public ast_term {
    ast_expression expression;
    explicit ast_term_expression(ast_expression expression)
    : ast_term(type_t::EXPRESSION),
      expression(std::move(expression)) {};
};

struct ast_term_unary : public ast_term {
    ast_unary_op op;
    ast_term* term;

    ast_term_unary(ast_unary_op op, ast_term *term) : ast_term(type_t::UNARY), term(term), op(op) {};
};

struct ast_term_subroutine_call : public ast_term {
    ast_subroutine_call call;
    explicit ast_term_subroutine_call(ast_subroutine_call call)
    : ast_term(ast_term::type_t::SUBROUTINE_CALL),
    call(std::move(call)) {};
};


struct ast_statement {
    enum struct type_t {
        LET,
        IF,
        WHILE,
        DO,
        RETURN
    };

    type_t type;

    explicit ast_statement(type_t type) : type(type) {};
};

struct ast_statement_let : public ast_statement {
    std::optional<ast_expression> array_access = std::nullopt;
    std::string identifier;
    ast_expression assignment;

    ast_statement_let()
    : ast_statement(type_t::LET){};
};

struct ast_statement_if : public ast_statement {
    ast_expression conditional;
    std::list<ast_statement*> true_statements;
    std::list<ast_statement*> false_statements;

    explicit ast_statement_if(ast_expression conditional)
        : ast_statement(ast_statement::type_t::IF),
        conditional(std::move(conditional)) {};
};

struct ast_statement_while : public ast_statement {
    ast_expression conditional;
    std::list<ast_statement*> statements;
    explicit ast_statement_while(ast_expression conditional)
        : ast_statement(ast_statement::type_t::WHILE),
        conditional(std::move(conditional)) {};
};

struct ast_statement_do : public ast_statement {
    ast_subroutine_call call;
    explicit ast_statement_do(ast_subroutine_call call)
    : ast_statement(ast_statement::type_t::DO),
    call(std::move(call)) {};
};

struct ast_statement_return : public ast_statement {
    ast_expression value;
    explicit ast_statement_return(ast_expression value)
    : ast_statement(ast_statement::type_t::RETURN),
    value(std::move(value)) {};
};

struct ast_class_subroutine {
    enum struct type_t {
        METHOD,
        FUNCTION,
        CONSTRUCTOR
    };

    type_t type;
    std::string return_type;
    std::string identifier;
    std::list<ast_parameter> parameters;
    std::list<ast_subroutine_local> locals;
    std::list<ast_statement*> statements;
};

struct ast_class {
    std::string identifier;
    std::list<ast_class_variable> variables;
    std::list<ast_class_subroutine> subroutines;
};