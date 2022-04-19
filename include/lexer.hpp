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

    token _expect_token(token::type_t expected_type);
    token _expect_token(token::type_t expected_type, const token::value_t& expected_value);
    token _expect_subroutine();
    token _expect_type();
    token _expect_type_voidable();

    void _compile_class();
    void _compile_class_subroutine_declaration();
    void _compile_class_variable_declaration();
    void _compile_class_subroutine_parameter_list();
    void _compile_class_subroutine_body();
    void _compile_variable_declarations();
    void _compile_statements();
    void _compile_let_statement();
    void _compile_if_statement();
    void _compile_while_statement();
    void _compile_do_statement();
    void _compile_return_statement();
    void _compile_expression();
};
