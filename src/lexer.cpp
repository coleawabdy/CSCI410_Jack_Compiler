#include "lexer.hpp"

#if N2T_COMPLIANT == 10
    #define XML_ELEMENT(name) _parent_element->InsertEndChild(_document->NewElement(name))
    #define XML_ELEMENT_TEXT(name, text) _parent_element->InsertEndChild(_document->NewElement(name))->ToElement()->InsertNewText(text)
    #define XML_ELEMENT_TOKEN(tk) XML_ELEMENT_TEXT(token::type_to_string(tk.type).c_str(), token::to_string(tk).c_str())
    #define XML_ENTER(element) _parent_element = element
    #define XML_EXIT _parent_element = _parent_element->Parent();
#else
    #define XML_ELEMENT_TEXT(name, text)
    #define XML_ELEMENT(name)
    #define XML_ENTER(element)
    #define XML_EXIT
    #define XML_ELEMENT_TOKEN(tk) tk
#endif

void lexer::run(tokenizer &tokenizer) {
    _tokenizer = &tokenizer;
    _tokenizer->reset();
#if N2T_COMPLIANT == 10
    _document->Clear();
#endif

    _compile_class();
}

void lexer::_compile_class_variable_declaration() {

}

bool lexer::_check_token(token::type_t type) {
    return _tokenizer->peek().type == type;
}

bool lexer::_check_token(token::type_t type, const token::value_t &value) {
    auto peek = _tokenizer->peek();
    bool q = peek.type == type && peek.value == value;
    return q;
}

bool lexer::_check_subroutine() {
    auto peek = _tokenizer->peek();
    if(peek.type == token::type_t::KEYWORD) {
        auto value = peek.get_value<token::keyword_t>();
        return value == token::keyword_t::FUNCTION || value == token::keyword_t::METHOD || value == token::keyword_t::CONSTRUCTOR;
    }
    return false;
}

bool lexer::_check_type() {
    auto peek = _tokenizer->peek();
    if(peek.type == token::type_t::KEYWORD) {
        auto value = peek.get_value<token::keyword_t>();
        return value == token::keyword_t::INT || value == token::keyword_t::CHAR || value == token::keyword_t::BOOL;
    } else if(peek.type == token::type_t::IDENTIFIER)
        return true;

    return false;
}

bool lexer::_check_type_voidable() {
    auto peek = _tokenizer->peek();
    if(peek.type == token::type_t::KEYWORD) {
        auto value = peek.get_value<token::keyword_t>();
        return value == token::keyword_t::INT || value == token::keyword_t::CHAR || value == token::keyword_t::BOOL || value == token::keyword_t::VOID;
    } else if(peek.type == token::type_t::IDENTIFIER)
        return true;

    return false;
}

token lexer::_expect_token(token::type_t expected_type) {
    if(!_check_token(expected_type))
        throw std::runtime_error("Expected token '" + token::type_to_string(expected_type) + "' got '" + token::type_to_string(_tokenizer->peek().type) + "'");

    return _tokenizer->next();
}

token lexer::_expect_token(token::type_t expected_type, const token::value_t& expected_value) {
    auto expected_token = token {
        .type   = expected_type,
        .value  = expected_value
    };

    if(!_check_token(expected_type, expected_value))
        throw std::runtime_error("Expected token value '" + token::to_string(expected_token) + "' got '" + token::to_string(_tokenizer->peek()) + "'");

    return _tokenizer->next();
}

token lexer::_expect_subroutine() {
    if(!_check_subroutine())
        throw std::runtime_error("Expected a subroutine declaration, got '" + token::to_string(_tokenizer->peek()));
    return _tokenizer->next();
}

token lexer::_expect_type() {
    if(!_check_type())
        throw std::runtime_error("Expected a type");

    return _tokenizer->next();
}

token lexer::_expect_type_voidable() {
    if(!_check_type_voidable())
        throw std::runtime_error("Expected a type but got '" + token::to_string(_tokenizer->peek()) + "'");

    return _tokenizer->next();
}

void lexer::_compile_class() {
#if N2T_COMPLIANT == 10
    if (_parent_element == nullptr) {
        _parent_element = _document->NewElement("class");
        _document->InsertEndChild(_parent_element);
    }
    else
        XML_ENTER(XML_ELEMENT("class"));
#endif
    _expect_token(token::type_t::KEYWORD, token::keyword_t::CLASS);
    XML_ELEMENT_TEXT("keyword", "class");

    auto class_identifier = _expect_token(token::type_t::IDENTIFIER);
    XML_ELEMENT_TEXT("identifier", class_identifier.get_value<token::identifier_t>().c_str());

    _expect_token(token::type_t::SYMBOL, '{');
    XML_ELEMENT_TEXT("symbol", "{");


    while (true) {
        auto peek = _tokenizer->peek();
        if (peek.type == token::type_t::KEYWORD) {
            auto val = peek.get_value<token::keyword_t>();
            if (val == token::keyword_t::FUNCTION || val == token::keyword_t::CONSTRUCTOR ||
                val == token::keyword_t::METHOD)
                _compile_class_subroutine_declaration();
            else if (val == token::keyword_t::STATIC || val == token::keyword_t::FIELD)
                _compile_class_variable_declaration();
            else
                break;
        } else
            break;
    }

    _expect_token(token::type_t::SYMBOL, '}');
    XML_ELEMENT_TEXT("symbol", "}");
}

void lexer::_compile_class_subroutine_declaration() {
    XML_ENTER(XML_ELEMENT("subroutineDec"));

    auto subroutine = _expect_subroutine();
    XML_ELEMENT_TEXT("keyword", token::keyword_to_string(subroutine.get_value<token::keyword_t>()).c_str());

    auto return_type = _expect_type_voidable();
    if (return_type.type == token::type_t::IDENTIFIER) {
        XML_ELEMENT_TEXT("identifier", return_type.get_value<token::identifier_t>().c_str());
    } else {
        XML_ELEMENT_TEXT("keyword", "void");
    }

    auto identifier = _expect_token(token::type_t::IDENTIFIER);
    XML_ELEMENT_TEXT("identifier", identifier.get_value<token::identifier_t>().c_str());

    _compile_class_subroutine_parameter_list();
    _compile_class_subroutine_body();

    XML_EXIT
}

void lexer::_compile_class_subroutine_parameter_list() {
    _expect_token(token::type_t::SYMBOL, '(');
    XML_ELEMENT_TEXT("symbol", "(");

    XML_ENTER(XML_ELEMENT("parameterList"));

    bool need_next = false;
    while(true) {

        if(_check_token(token::type_t::SYMBOL, ')')) {
            if(need_next)
                throw std::runtime_error("Expected another parameter");
            else
                break;
        }

        auto type = _expect_type();
        XML_ELEMENT_TEXT(token::type_to_string(type.type).c_str(), token::to_string(type).c_str());

        auto name = _expect_token(token::type_t::IDENTIFIER);
        XML_ELEMENT_TEXT("identifier", name.get_value<token::identifier_t>().c_str());

        if(!_check_token(token::type_t::SYMBOL, ','))
            break;

        _expect_token(token::type_t::SYMBOL, ',');
        XML_ELEMENT_TEXT("symbol", ",");
        need_next = true;
    }

#if N2T_COMPLIANT == 10
    if(_parent_element->NoChildren())
        _parent_element->ToElement()->InsertNewText("");
#endif

    XML_EXIT

    _expect_token(token::type_t::SYMBOL, ')');
    XML_ELEMENT_TEXT("symbol", ")");
}

void lexer::_compile_class_subroutine_body() {
    XML_ENTER(XML_ELEMENT("subroutineBody"));

    _expect_token(token::type_t::SYMBOL, '{');
    XML_ELEMENT_TEXT("symbol", "{");

    _compile_variable_declarations();
    _compile_statements();

    _expect_token(token::type_t::SYMBOL, '}');
    XML_ELEMENT_TEXT("symbol", "}");

    XML_EXIT
}

void lexer::_compile_variable_declarations() {
    while(_check_token(token::type_t::KEYWORD, token::keyword_t::VAR)) {
        XML_ENTER(XML_ELEMENT("varDec"));

        _expect_token(token::type_t::KEYWORD, token::keyword_t::VAR);
        XML_ELEMENT_TEXT("keyword", "var");

        auto type = _expect_type();
        XML_ELEMENT_TEXT(token::type_to_string(type.type).c_str(), token::to_string(type).c_str());

        while(true) {
            auto identifier = _expect_token(token::type_t::IDENTIFIER);
            XML_ELEMENT_TEXT("identifier", identifier.get_value<token::identifier_t>().c_str());

            if(_check_token(token::type_t::SYMBOL, ','))
                _expect_token(token::type_t::SYMBOL, ',');
            else
                break;
        }

        _expect_token(token::type_t::SYMBOL, ';');
        XML_ELEMENT_TEXT("symbol", ";");

        XML_EXIT
    }
}

void lexer::_compile_statements() {
    XML_ENTER(XML_ELEMENT("statements"));

    while(true) {
        if(_check_token(token::type_t::KEYWORD, "let"))
            _compile_let_statement();
        else if(_check_token(token::type_t::KEYWORD, "if"))
            _compile_if_statement();
        else if(_check_token(token::type_t::KEYWORD, "while"))
            _compile_while_statement();
        else if(_check_token(token::type_t::KEYWORD, "do"))
            _compile_do_statement();
        else if(_check_token(token::type_t::KEYWORD, "return"))
            _compile_return_statement();
        else
            break;
    }

    XML_EXIT
}

void lexer::_compile_let_statement() {
    XML_ENTER(XML_ELEMENT("letStatement"));

    _expect_token(token::type_t::KEYWORD, token::keyword_t::LET);
    XML_ELEMENT_TEXT("keyword", "let");

    auto identifier = _expect_token(token::type_t::IDENTIFIER);
    XML_ELEMENT_TOKEN(identifier);

    if(_check_token(token::type_t::SYMBOL, '[')) {
        _expect_token(token::type_t::SYMBOL, '[');
        XML_ELEMENT_TEXT("symbol", "[");

        _compile_expression();

        _expect_token(token::type_t::SYMBOL, ']');
        XML_ELEMENT_TEXT("symbol", "]");
    }

    XML_ELEMENT_TOKEN(_expect_token(token::type_t::SYMBOL, '='));

    _compile_expression();

    XML_ELEMENT_TOKEN(_expect_token(token::type_t::SYMBOL, ';'));

    XML_EXIT
}

void lexer::_compile_if_statement() {
    XML_ENTER(XML_ELEMENT("ifStatement"));



    XML_EXIT
}

void lexer::_compile_while_statement() {
    XML_ENTER(XML_ELEMENT("whileStatement"));



    XML_EXIT
}

void lexer::_compile_do_statement() {
    XML_ENTER(XML_ELEMENT("doStatement"));



    XML_EXIT
}

void lexer::_compile_return_statement() {
    XML_ENTER(XML_ELEMENT("returnStatement"));



    XML_EXIT
}

void lexer::_compile_expression() {

}
