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

    _parse_class();
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

bool lexer::_check_op() {
    auto peek = _tokenizer->peek();
    if(peek.type == token::type_t::SYMBOL) {
        auto val = peek.get_value<token::symbol_t>();
        return val == '+' || val == '-' || val == '*' || val == '/' || val == '&' || val == '|' || val == '<' || val == '>' || val == '=';
    }

    return false;
}

bool lexer::_check_unary_op() {
    auto peek = _tokenizer->peek();
    if(peek.type == token::type_t::SYMBOL) {
        auto val = peek.get_value<token::symbol_t>();
        return val == '-' || val == '~';
    }

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

token lexer::_expect_op() {
    if(!_check_op())
        throw std::runtime_error("Expected an operator");

    return _tokenizer->next();
}

token lexer::_expect_unary_op() {
    if(!_check_unary_op())
        throw std::runtime_error("Expected unary op");

    return _tokenizer->next();
}

void lexer::_parse_class() {
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
                _parse_subroutine_declaration();
            else if (val == token::keyword_t::STATIC || val == token::keyword_t::FIELD)
                _parse_class_variable_declaration();
            else
                break;
        } else
            break;
    }

    _expect_token(token::type_t::SYMBOL, '}');
    XML_ELEMENT_TEXT("symbol", "}");
}

void lexer::_parse_subroutine_declaration() {
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

    _parse_class_subroutine_parameter_list();
    _parse_class_subroutine_body();

    XML_EXIT
}

void lexer::_parse_class_subroutine_parameter_list() {
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
        _parent_element->ToElement()->InsertNewText("\n");
#endif

    XML_EXIT

    _expect_token(token::type_t::SYMBOL, ')');
    XML_ELEMENT_TEXT("symbol", ")");
}

void lexer::_parse_class_subroutine_body() {
    XML_ENTER(XML_ELEMENT("subroutineBody"));

    _expect_token(token::type_t::SYMBOL, '{');
    XML_ELEMENT_TEXT("symbol", "{");

    _parse_variable_declarations();
    _parse_statements();

    _expect_token(token::type_t::SYMBOL, '}');
    XML_ELEMENT_TEXT("symbol", "}");

    XML_EXIT
}

void lexer::_parse_class_variable_declaration() {
    XML_ENTER(XML_ELEMENT("classVarDec"));

    if(_check_token(token::type_t::KEYWORD, token::keyword_t::STATIC)) {
        _expect_token(token::type_t::KEYWORD, token::keyword_t::STATIC);
        XML_ELEMENT_TEXT("keyword", "static");
    } else if(_check_token(token::type_t::KEYWORD, token::keyword_t::FIELD)) {
        _expect_token(token::type_t::KEYWORD, token::keyword_t::FIELD);
        XML_ELEMENT_TEXT("keyword", "field");
    }

    auto type = _expect_type();
    XML_ELEMENT_TEXT(token::type_to_string(type.type).c_str(), token::to_string(type).c_str());

    auto identifier = _expect_token(token::type_t::IDENTIFIER);
    XML_ELEMENT_TEXT("identifier", token::to_string(identifier).c_str());

    while(_check_token(token::type_t::SYMBOL, ',')) {
        _expect_token(token::type_t::SYMBOL, ',');
        XML_ELEMENT_TEXT("symbol", ",");

        identifier = _expect_token(token::type_t::IDENTIFIER);
        XML_ELEMENT_TEXT("identifier", token::to_string(identifier).c_str());
    }

    _expect_token(token::type_t::SYMBOL, ';');
    XML_ELEMENT_TEXT("symbol", ";");

    XML_EXIT
}

void lexer::_parse_variable_declarations() {
    while(_check_token(token::type_t::KEYWORD, token::keyword_t::VAR)) {
        XML_ENTER(XML_ELEMENT("varDec"));

        _expect_token(token::type_t::KEYWORD, token::keyword_t::VAR);
        XML_ELEMENT_TEXT("keyword", "var");

        auto type = _expect_type();
        XML_ELEMENT_TEXT(token::type_to_string(type.type).c_str(), token::to_string(type).c_str());

        while(true) {
            auto identifier = _expect_token(token::type_t::IDENTIFIER);
            XML_ELEMENT_TEXT("identifier", identifier.get_value<token::identifier_t>().c_str());

            if (_check_token(token::type_t::SYMBOL, ',')) {
                _expect_token(token::type_t::SYMBOL, ',');
            XML_ELEMENT_TEXT("symbol", ",");
            }
            else
                break;
        }

        _expect_token(token::type_t::SYMBOL, ';');
        XML_ELEMENT_TEXT("symbol", ";");

        XML_EXIT
    }
}

void lexer::_parse_statements() {
    XML_ENTER(XML_ELEMENT("statements"));

    while(true) {
        if(_check_token(token::type_t::KEYWORD, token::keyword_t::LET))
            _parse_let_statement();
        else if(_check_token(token::type_t::KEYWORD, token::keyword_t::IF))
            _parse_if_statement();
        else if(_check_token(token::type_t::KEYWORD, token::keyword_t::WHILE))
            _parse_while_statement();
        else if(_check_token(token::type_t::KEYWORD, token::keyword_t::DO))
            _parse_do_statement();
        else if(_check_token(token::type_t::KEYWORD, token::keyword_t::RETURN))
            _parse_return_statement();
        else
            break;
    }

#if N2T_COMPLIANT == 10
    if(_parent_element->NoChildren())
        _parent_element->ToElement()->InsertNewText("\n");
#endif

    XML_EXIT
}

void lexer::_parse_let_statement() {
    XML_ENTER(XML_ELEMENT("letStatement"));

    _expect_token(token::type_t::KEYWORD, token::keyword_t::LET);
    XML_ELEMENT_TEXT("keyword", "let");

    auto identifier = _expect_token(token::type_t::IDENTIFIER);
    XML_ELEMENT_TOKEN(identifier);

    if(_check_token(token::type_t::SYMBOL, '[')) {
        _expect_token(token::type_t::SYMBOL, '[');
        XML_ELEMENT_TEXT("symbol", "[");

        _parse_expression();

        _expect_token(token::type_t::SYMBOL, ']');
        XML_ELEMENT_TEXT("symbol", "]");
    }

    _expect_token(token::type_t::SYMBOL, '=');
    XML_ELEMENT_TEXT("symbol", "=");

    _parse_expression();

    _expect_token(token::type_t::SYMBOL, ';');
    XML_ELEMENT_TEXT("symbol", ";");

    XML_EXIT
}

void lexer::_parse_if_statement() {
    XML_ENTER(XML_ELEMENT("ifStatement"));

    _expect_token(token::type_t::KEYWORD, token::keyword_t::IF);
    XML_ELEMENT_TEXT("keyword", "if");

    _expect_token(token::type_t::SYMBOL, '(');
    XML_ELEMENT_TEXT("symbol", "(");

    _parse_expression();

    _expect_token(token::type_t::SYMBOL, ')');
    XML_ELEMENT_TEXT("symbol", ")");

    _expect_token(token::type_t::SYMBOL, '{');
    XML_ELEMENT_TEXT("symbol", "{");

    _parse_statements();

    _expect_token(token::type_t::SYMBOL, '}');
    XML_ELEMENT_TEXT("symbol", "}");

    if(_check_token(token::type_t::KEYWORD, token::keyword_t::ELSE)) {
        _expect_token(token::type_t::KEYWORD, token::keyword_t::ELSE);
        XML_ELEMENT_TEXT("keyword", "else");

        _expect_token(token::type_t::SYMBOL, '{');
        XML_ELEMENT_TEXT("symbol", "{");

        _parse_statements();

        _expect_token(token::type_t::SYMBOL, '}');
        XML_ELEMENT_TEXT("symbol", "}");
    }

    XML_EXIT
}

void lexer::_parse_while_statement() {
    XML_ENTER(XML_ELEMENT("whileStatement"));

    _expect_token(token::type_t::KEYWORD, token::keyword_t::WHILE);
    XML_ELEMENT_TEXT("keyword", "while");

    _expect_token(token::type_t::SYMBOL, '(');
    XML_ELEMENT_TEXT("symbol", "(");

    _parse_expression();

    _expect_token(token::type_t::SYMBOL, ')');
    XML_ELEMENT_TEXT("symbol", ")");

    _expect_token(token::type_t::SYMBOL, '{');
    XML_ELEMENT_TEXT("symbol", "{");

    _parse_statements();

    _expect_token(token::type_t::SYMBOL, '}');
    XML_ELEMENT_TEXT("symbol", "}");

    XML_EXIT
}

void lexer::_parse_do_statement() {
    XML_ENTER(XML_ELEMENT("doStatement"));

    _expect_token(token::type_t::KEYWORD, token::keyword_t::DO);
    XML_ELEMENT_TEXT("keyword", "do");

    _parse_subroutine_call();

    _expect_token(token::type_t::SYMBOL, ';');
    XML_ELEMENT_TEXT("symbol", ";");

    XML_EXIT
}

void lexer::_parse_return_statement() {
    XML_ENTER(XML_ELEMENT("returnStatement"));

    _expect_token(token::type_t::KEYWORD, token::keyword_t::RETURN);
    XML_ELEMENT_TEXT("keyword", "return");

    if(!_check_token(token::type_t::SYMBOL, ';'))
        _parse_expression();

    _expect_token(token::type_t::SYMBOL, ';');
    XML_ELEMENT_TEXT("symbol", ";");

    XML_EXIT
}

void lexer::_parse_expression() {
    XML_ENTER(XML_ELEMENT("expression"));

    _parse_term();

    while(true) {
        if(!_check_op())
            break;

        auto op = _expect_op();
        XML_ELEMENT_TEXT("symbol", token::to_string(op).c_str());

        _parse_term();
    }

    XML_EXIT
}

void lexer::_parse_subroutine_call() {
    auto identifier = _expect_token(token::type_t::IDENTIFIER);
    XML_ELEMENT_TEXT("identifier", token::to_string(identifier).c_str());

    if(_check_token(token::type_t::SYMBOL, '.')) {
        _expect_token(token::type_t::SYMBOL, '.');
        XML_ELEMENT_TEXT("symbol", ".");

        auto iden2 = _expect_token(token::type_t::IDENTIFIER);
        XML_ELEMENT_TEXT("identifier", token::to_string(iden2).c_str());
    }

    _expect_token(token::type_t::SYMBOL, '(');
    XML_ELEMENT_TEXT("symbol", "(");

    XML_ENTER(XML_ELEMENT("expressionList"));
    if(!_check_token(token::type_t::SYMBOL, ')')) {
        _parse_expression();

        while (_check_token(token::type_t::SYMBOL, ',')) {
            _expect_token(token::type_t::SYMBOL, ',');
            XML_ELEMENT_TEXT("symbol", ",");

            _parse_expression();
        }
    }
#if N2T_COMPLIANT == 10
    else  {
        _parent_element->ToElement()->InsertNewText("\n");
    }
#endif
    XML_EXIT

    _expect_token(token::type_t::SYMBOL, ')');
    XML_ELEMENT_TEXT("symbol", ")");
}

void lexer::_parse_term() {
    XML_ENTER(XML_ELEMENT("term"));

    if(_check_token(token::type_t::INT_CONSTANT)) {
        auto c = _expect_token(token::type_t::INT_CONSTANT);
        XML_ELEMENT_TEXT("integerConstant", token::to_string(c).c_str());
    } else if(_check_token(token::type_t::STRING_CONSTANT)) {
        auto str = _expect_token(token::type_t::STRING_CONSTANT);
        XML_ELEMENT_TEXT("stringConstant", token::to_string(str).c_str());
    } else if(_check_token(token::type_t::KEYWORD, token::keyword_t::FALSE)
            || _check_token(token::type_t::KEYWORD, token::keyword_t::TRUE)
            || _check_token(token::type_t::KEYWORD, token::keyword_t::NUL)
            || _check_token(token::type_t::KEYWORD, token::keyword_t::THIS)) {
        auto keyword = _expect_token(token::type_t::KEYWORD);
        XML_ELEMENT_TEXT("keyword", token::to_string(keyword).c_str());
    } else if(_check_token(token::type_t::IDENTIFIER)) {
        auto peek1 = _tokenizer->peek(1);
        if(peek1.type == token::type_t::SYMBOL && peek1.get_value<token::symbol_t>() == '(' || peek1.get_value<token::symbol_t>() == '.') {
            _parse_subroutine_call();
        } else {
            auto identifier = _expect_token(token::type_t::IDENTIFIER);
            XML_ELEMENT_TEXT("identifier", token::to_string(identifier).c_str());

            if(_check_token(token::type_t::SYMBOL, '[')) {
                _expect_token(token::type_t::SYMBOL, '[');
                XML_ELEMENT_TEXT("symbol", "[");

                _parse_expression();

                _expect_token(token::type_t::SYMBOL, ']');
                XML_ELEMENT_TEXT("symbol", "]");
            }
        }
    } else if(_check_token(token::type_t::SYMBOL, '(')) {
        _expect_token(token::type_t::SYMBOL, '(');
        XML_ELEMENT_TEXT("symbol", "(");

        _parse_expression();

        _expect_token(token::type_t::SYMBOL, ')');
        XML_ELEMENT_TEXT("symbol", ")");
    } else if(_check_unary_op()) {
        auto op = _expect_unary_op();
        XML_ELEMENT_TEXT("symbol", token::to_string(op).c_str());

        _parse_term();
    } else
        throw std::runtime_error("No valid term could be found");
    XML_EXIT
}