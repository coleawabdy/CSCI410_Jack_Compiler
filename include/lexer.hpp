#pragma once

#include "tokenizer.hpp"

#if N2T_COMPLIANT == 10
    #include <tinyxml2.h>
#endif

class lexer {
private:
    tokenizer* _tokenizer = nullptr;
#if N2T_COMPLIANT == 10
private:
    tinyxml2::XMLDocument* _document = new tinyxml2::XMLDocument();
    tinyxml2::XMLNode* _parent_element = nullptr;
public:
    ~lexer() { delete _document; }

    tinyxml2::XMLDocument* get_document() { return _document; }
#else
public:
    ~lexer() = default;
#endif
    lexer() = default;

    void run(tokenizer& tokenizer);
private:
    bool _check_token(token::type_t type);
    bool _check_token(token::type_t type, const token::value_t& value);
    bool _check_subroutine();
    bool _check_type();
    bool _check_type_voidable();
    bool _check_op();
    bool _check_unary_op();

    token _expect_token(token::type_t expected_type);
    token _expect_token(token::type_t expected_type, const token::value_t& expected_value);
    token _expect_subroutine();
    token _expect_type();
    token _expect_type_voidable();
    token _expect_op();
    token _expect_unary_op();

    void _parse_class();
    void _parse_subroutine_declaration();
    void _parse_class_subroutine_parameter_list();
    void _parse_class_subroutine_body();
    void _parse_class_variable_declaration();
    void _parse_variable_declarations();
    void _parse_statements();
    void _parse_let_statement();
    void _parse_if_statement();
    void _parse_while_statement();
    void _parse_do_statement();
    void _parse_return_statement();
    void _parse_expression();
    void _parse_subroutine_call();
    void _parse_term();
};
