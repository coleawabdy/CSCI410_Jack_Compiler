#pragma once

#include "generator.hpp"
#include "tokenizer.hpp"
#include "ast.hpp"

class lexer {
private:
    tokenizer* _tokenizer = nullptr;
    ast_class* _class;
public:
    ~lexer() = default;
    lexer() = default;

    void run(tokenizer& tokenizer);

    [[nodiscard]] ast_class* get_class() const { return _class; };
private:
    bool _check_token(token::type_t type);
    bool _check_token(token::type_t type, const token::value_t& value);
    bool _check_subroutine();
    bool _check_type();
    bool _check_type_voidable();
    bool _check_op();
    bool _check_unary_op();
    bool _check_class_variable_declaration();

    token _expect_token(token::type_t expected_type);
    token _expect_token(token::type_t expected_type, const token::value_t& expected_value);
    token _expect_subroutine();
    token _expect_type();
    token _expect_type_voidable();
    token _expect_op();
    token _expect_unary_op();

    ast_class* _parse_class();
    ast_class_subroutine _parse_class_subroutine_declaration();
    ast_statement* _parse_statement();
    ast_statement_let * _parse_let_statement();
    ast_statement_if * _parse_if_statement();
    ast_statement_while * _parse_while_statement();
    ast_statement_do * _parse_do_statement();
    ast_statement_return * _parse_return_statement();
    ast_expression _parse_expression();
    ast_subroutine_call _parse_subroutine_call();
    ast_term * _parse_term();
    void _parse_statements(std::list<ast_statement *> &statements);

    static ast_binary_op _binary_op_from_token(const token& token);
    static ast_unary_op _unary_op_from_token(const token& token);
};
