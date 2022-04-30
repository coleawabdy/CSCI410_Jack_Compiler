#include "lexer.hpp"

// This is a recursive parser, disable recursion check
// NOLINTBEGIN(misc-no-recursion)

void lexer::run(tokenizer &tokenizer) {
    _tokenizer = &tokenizer;
    _tokenizer->reset();

    _class = _parse_class();
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

bool lexer::_check_class_variable_declaration() {
    auto peek = _tokenizer->peek();
    if(peek.type == token::type_t::KEYWORD) {
        auto val = peek.get_value<token::keyword_t>();
        return val == token::keyword_t::FIELD || val == token::keyword_t::STATIC;
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

ast_class * lexer::_parse_class() {
    auto cl = new ast_class();

    _expect_token(token::type_t::KEYWORD, token::keyword_t::CLASS);

    cl->identifier = token::to_string(_expect_token(token::type_t::IDENTIFIER));
    _expect_token(token::type_t::SYMBOL, '{');

    while (_check_class_variable_declaration()) {
        ast_class_variable var;

        auto dec_keyword = _expect_token(token::type_t::KEYWORD);
        var.is_static = dec_keyword.get_value<token::keyword_t>() == token::keyword_t::STATIC;

        auto type_token = _expect_type();
        var.type = token::to_string(type_token);

        auto identifier = _expect_token(token::type_t::IDENTIFIER);
        var.identifiers.push_back(token::to_string(identifier));

        while(_check_token(token::type_t::SYMBOL, ',')) {
            _expect_token(token::type_t::SYMBOL, ',');
            identifier = _expect_token(token::type_t::IDENTIFIER);
            var.identifiers.push_back(token::to_string(identifier));
        }

        _expect_token(token::type_t::SYMBOL, ';');

        cl->variables.push_back(var);
    }

    while(_check_subroutine())
        cl->subroutines.push_back(_parse_class_subroutine_declaration());

    _expect_token(token::type_t::SYMBOL, '}');

    return cl;
}

ast_class_subroutine lexer::_parse_class_subroutine_declaration() {
    ast_class_subroutine subroutine;

    auto kw = _expect_subroutine().get_value<token::keyword_t>();
    switch(kw) {
        case token::keyword_t::CONSTRUCTOR:
            subroutine.type = ast_class_subroutine::type_t::CONSTRUCTOR;
            break;
        case token::keyword_t::METHOD:
            subroutine.type = ast_class_subroutine::type_t::METHOD;
            break;
        case token::keyword_t::FUNCTION:
            subroutine.type = ast_class_subroutine::type_t::FUNCTION;
            break;
        default:
            break;
    }

    subroutine.return_type = token::to_string(_expect_type_voidable());
    subroutine.identifier = token::to_string(_expect_token(token::type_t::IDENTIFIER));

    _expect_token(token::type_t::SYMBOL, '(');

    if(_check_type()) {
        ast_parameter param;
        param.type = token::to_string(_expect_type());
        param.identifier = token::to_string(_expect_token(token::type_t::IDENTIFIER));

        subroutine.parameters.push_back(param);

        while(_check_token(token::type_t::SYMBOL, ',')) {
            _expect_token(token::type_t::SYMBOL, ',');
            param.type = token::to_string(_expect_type());
            param.identifier = token::to_string(_expect_token(token::type_t::IDENTIFIER));
            subroutine.parameters.push_back(param);
        }
    }

    _expect_token(token::type_t::SYMBOL, ')');
    _expect_token(token::type_t::SYMBOL, '{');

    while(_check_token(token::type_t::KEYWORD, token::keyword_t::VAR)) {
        ast_subroutine_local local;

        _expect_token(token::type_t::KEYWORD, token::keyword_t::VAR);
        local.type = token::to_string(_expect_type());
        local.identifiers.push_back(token::to_string(_expect_token(token::type_t::IDENTIFIER)));
        while(_check_token(token::type_t::SYMBOL, ',')) {
            _expect_token(token::type_t::SYMBOL, ',');

            local.identifiers.push_back(token::to_string(_expect_token(token::type_t::IDENTIFIER)));
        }
        _expect_token(token::type_t::SYMBOL, ';');

        subroutine.locals.push_back(local);
    }

    _parse_statements(subroutine.statements);

    _expect_token(token::type_t::SYMBOL, '}');

    return subroutine;
}

ast_statement *lexer::_parse_statement() {
    if(_check_token(token::type_t::KEYWORD)) {
        auto val = _tokenizer->peek().get_value<token::keyword_t>();
        switch(val) {
            case token::keyword_t::LET:
                return _parse_let_statement();
            case token::keyword_t::IF:
                return _parse_if_statement();
            case token::keyword_t::WHILE:
                return _parse_while_statement();
            case token::keyword_t::DO:
                return _parse_do_statement();
            case token::keyword_t::RETURN:
                return _parse_return_statement();
            default:
                break;
        }
    }

    return nullptr;
}

ast_statement_let * lexer::_parse_let_statement() {
    auto statement = new ast_statement_let();

    _expect_token(token::type_t::KEYWORD, token::keyword_t::LET);

    statement->identifier = token::to_string(_expect_token(token::type_t::IDENTIFIER));

    if(_check_token(token::type_t::SYMBOL, '[')) {
        _expect_token(token::type_t::SYMBOL, '[');
        statement->array_access = _parse_expression();
        _expect_token(token::type_t::SYMBOL, ']');
    }

    _expect_token(token::type_t::SYMBOL, '=');

    statement->assignment = _parse_expression();

    _expect_token(token::type_t::SYMBOL, ';');
    return statement;
}

ast_statement_if * lexer::_parse_if_statement() {
    _expect_token(token::type_t::KEYWORD, token::keyword_t::IF);
    _expect_token(token::type_t::SYMBOL, '(');

    auto statement = new ast_statement_if(_parse_expression());

    _expect_token(token::type_t::SYMBOL, ')');
    _expect_token(token::type_t::SYMBOL, '{');

    _parse_statements(statement->true_statements);

    _expect_token(token::type_t::SYMBOL, '}');

    if(_check_token(token::type_t::KEYWORD, token::keyword_t::ELSE)) {
        _expect_token(token::type_t::KEYWORD, token::keyword_t::ELSE);

        _expect_token(token::type_t::SYMBOL, '{');

        _parse_statements(statement->false_statements);

        _expect_token(token::type_t::SYMBOL, '}');
    }

    return statement;
}

ast_statement_while * lexer::_parse_while_statement() {
    _expect_token(token::type_t::KEYWORD, token::keyword_t::WHILE);

    _expect_token(token::type_t::SYMBOL, '(');
    auto while_statement = new ast_statement_while(_parse_expression());
    _expect_token(token::type_t::SYMBOL, ')');

    _expect_token(token::type_t::SYMBOL, '{');
    _parse_statements(while_statement->statements);
    _expect_token(token::type_t::SYMBOL, '}');

    return while_statement;
}

ast_statement_do * lexer::_parse_do_statement() {
    _expect_token(token::type_t::KEYWORD, token::keyword_t::DO);
    auto do_statement = new ast_statement_do(_parse_subroutine_call());
    _expect_token(token::type_t::SYMBOL, ';');

    return do_statement;
}

ast_statement_return * lexer::_parse_return_statement() {
    _expect_token(token::type_t::KEYWORD, token::keyword_t::RETURN);

    ast_statement_return* ret_statement = nullptr;
    if(!_check_token(token::type_t::SYMBOL, ';'))
        ret_statement = new ast_statement_return(_parse_expression());
    else
        ret_statement = new ast_statement_return(ast_expression(new ast_term_integer(0)));

    _expect_token(token::type_t::SYMBOL, ';');

    return ret_statement;
}

ast_expression lexer::_parse_expression() {
    ast_expression expr(_parse_term());

    while(_check_op()) {
        ast_expression::op_term_t term;
        term.first = _binary_op_from_token(_expect_op());
        term.second = _parse_term();

        expr.secondaries.push_back(term);
    }

    return expr;
}

ast_subroutine_call lexer::_parse_subroutine_call() {
    ast_subroutine_call call(token::to_string(_expect_token(token::type_t::IDENTIFIER)));

    if(_check_token(token::type_t::SYMBOL, '.')) {
        _expect_token(token::type_t::SYMBOL, '.');

        call.callee_identifier = call.subroutine_identifier;
        call.subroutine_identifier = token::to_string(_expect_token(token::type_t::IDENTIFIER));
    }

    _expect_token(token::type_t::SYMBOL, '(');

    if(!_check_token(token::type_t::SYMBOL, ')')) {
        call.arguments.push_back(_parse_expression());

        while (_check_token(token::type_t::SYMBOL, ',')) {
            _expect_token(token::type_t::SYMBOL, ',');

            call.arguments.push_back(_parse_expression());
        }
    }

    _expect_token(token::type_t::SYMBOL, ')');

    return call;
}

ast_term * lexer::_parse_term() {
    ast_term* term;
    if(_check_token(token::type_t::INT_CONSTANT)) {
        auto value = _expect_token(token::type_t::INT_CONSTANT).get_value<token::int_constant_t>();
        term = new ast_term_integer(value);

    } else if(_check_token(token::type_t::STRING_CONSTANT)) {
        auto value = _expect_token(token::type_t::STRING_CONSTANT).get_value<token::string_constant_t>();
        term = new ast_term_string(value);

    } else if(_check_token(token::type_t::KEYWORD, token::keyword_t::FALSE)
            || _check_token(token::type_t::KEYWORD, token::keyword_t::TRUE)
            || _check_token(token::type_t::KEYWORD, token::keyword_t::NUL)
            || _check_token(token::type_t::KEYWORD, token::keyword_t::THIS)) {
        auto keyword = _expect_token(token::type_t::KEYWORD).get_value<token::keyword_t>();
        ast_term::type_t type;
        switch(keyword) {
            case token::keyword_t::NUL:
                type = ast_term::type_t::NUL;
                break;
            case token::keyword_t::THIS:
                type = ast_term::type_t::THIS;
                break;
            case token::keyword_t::TRUE:
                type = ast_term::type_t::TRUE;
                break;
            case token::keyword_t::FALSE:
                type = ast_term::type_t::FALSE;
                break;
            default:
                break;
        }
        term = new ast_term(type);

    } else if(_check_token(token::type_t::IDENTIFIER)) {
        auto peek = _tokenizer->peek(1);
        if(peek.type == token::type_t::SYMBOL && peek.get_value<token::symbol_t>() == '(' || peek.get_value<token::symbol_t>() == '.') {
            term = new ast_term_subroutine_call(_parse_subroutine_call());
        } else {
            auto identifier = token::to_string(_expect_token(token::type_t::IDENTIFIER));

            if(_check_token(token::type_t::SYMBOL, '[')) {
                _expect_token(token::type_t::SYMBOL, '[');

                auto array = new ast_term_array(_parse_expression());
                array->identifier = identifier;
                term = array;

                _expect_token(token::type_t::SYMBOL, ']');
            } else {
                term = new ast_term_variable(identifier);
            }
        }
    } else if(_check_token(token::type_t::SYMBOL, '(')) {
        _expect_token(token::type_t::SYMBOL, '(');

        term = new ast_term_expression(_parse_expression());

        _expect_token(token::type_t::SYMBOL, ')');
    } else if(_check_unary_op()) {
        term = new ast_term_unary(_unary_op_from_token(_expect_unary_op()), _parse_term());
    } else
        throw std::runtime_error("No valid term could be found");

    return term;
}

void lexer::_parse_statements(std::list<ast_statement *> &statements) {
    ast_statement* next_statement = nullptr;
    while((next_statement = _parse_statement()) != nullptr)
        statements.push_back(next_statement);
}

ast_binary_op lexer::_binary_op_from_token(const token &token) {
    if(token.type == token::type_t::SYMBOL) {
        auto value = token.get_value<token::symbol_t>();
        switch(value) {
            case '+':
                return ast_binary_op::ADD;
            case '-':
                return ast_binary_op::SUBTRACT;
            case '*':
                return ast_binary_op::MULTIPLY;
            case '/':
                return ast_binary_op::DIVIDE;
            case '&':
                return ast_binary_op::AND;
            case '|':
                return ast_binary_op::OR;
            case '>':
                return ast_binary_op::GREATER;
            case '<':
                return ast_binary_op::LESSER;
            case '=':
                return ast_binary_op::EQUAL;
            default:
                throw std::runtime_error("symbol is not an op");
        }
    } else
        throw std::runtime_error("provided token cannot be converted to op");
}

ast_unary_op lexer::_unary_op_from_token(const token &token) {
    if(token.type == token::type_t::SYMBOL) {
        auto value = token.get_value<token::symbol_t>();
        switch(value) {
            case '-':
                return ast_unary_op::NEGATE;
            case '~':
                return ast_unary_op::INVERT;
            default:
                throw std::runtime_error("symbol is not an op");
        }
    } else
        throw std::runtime_error("provided token cannot be converted to op");
}

// NOLINTEND(misc-no-recursion)