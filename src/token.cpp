#include "token.hpp"

#include <stdexcept>

token::keyword_t token::keyword_from_string(const std::string &str) {
    if(str == "class")
        return token::keyword_t::CLASS;
    else if(str == "method")
        return token::keyword_t::METHOD;
    else if(str == "function")
        return token::keyword_t::FUNCTION;
    else if(str == "constructor")
        return token::keyword_t::CONSTRUCTOR;
    else if(str == "int")
        return token::keyword_t::INT;
    else if(str == "boolean")
        return token::keyword_t::BOOL;
    else if(str == "char")
        return token::keyword_t::CHAR;
    else if(str == "void")
        return token::keyword_t::VOID;
    else if(str == "var")
        return token::keyword_t::VAR;
    else if(str == "static")
        return token::keyword_t::STATIC;
    else if(str == "field")
        return token::keyword_t::FIELD;
    else if(str == "let")
        return token::keyword_t::LET;
    else if(str == "do")
        return token::keyword_t::DO;
    else if(str == "if")
        return token::keyword_t::IF;
    else if(str == "else")
        return token::keyword_t::ELSE;
    else if(str == "while")
        return token::keyword_t::WHILE;
    else if(str == "return")
        return token::keyword_t::RETURN;
    else if(str == "true")
        return token::keyword_t::TRUE;
    else if(str == "false")
        return token::keyword_t::FALSE;
    else if(str == "null")
        return token::keyword_t::NUL;
    else if(str == "this")
        return token::keyword_t::THIS;
    else
        throw std::runtime_error("Unknown token: '" + str + "'");
}

std::string token::keyword_to_string(token::keyword_t keyword) {
    switch (keyword) {
        case token::keyword_t::CLASS:
            return "class";
        case token::keyword_t::METHOD:
            return "method";
        case token::keyword_t::FUNCTION:
            return "function";
        case token::keyword_t::CONSTRUCTOR:
            return "constructor";
        case token::keyword_t::INT:
            return "int";
        case token::keyword_t::BOOL:
            return "boolean";
        case token::keyword_t::CHAR:
            return "char";
        case token::keyword_t::VOID:
            return "void";
        case token::keyword_t::VAR:
            return "var";
        case token::keyword_t::STATIC:
            return "static";
        case token::keyword_t::FIELD:
            return "field";
        case token::keyword_t::LET:
            return "let";
        case token::keyword_t::DO:
            return "do";
        case token::keyword_t::IF:
            return "if";
        case token::keyword_t::ELSE:
            return "else";
        case token::keyword_t::WHILE:
            return "while";
        case token::keyword_t::RETURN:
            return "return";
        case token::keyword_t::TRUE:
            return "true";
        case token::keyword_t::FALSE:
            return "false";
        case token::keyword_t::NUL:
            return "null";
        case token::keyword_t::THIS:
            return "this";
        default:
            throw std::runtime_error("Failed to convert keyword to string");
    }
}

std::string token::type_to_string(token::type_t type) {
    switch(type) {
        case type_t::KEYWORD:
            return "keyword";
        case type_t::IDENTIFIER:
            return "identifier";
        case type_t::SYMBOL:
            return "symbol";
        case type_t::STRING_CONSTANT:
            return "stringConstant";
        case type_t::INT_CONSTANT:
            return "integerConstant";
    }
}

std::string token::to_string(const token &token) {
    std::string ret = " ";
    switch(token.type) {
        case type_t::KEYWORD:
            return keyword_to_string(token.get_value<keyword_t>());
        case type_t::IDENTIFIER:
            return token.get_value<identifier_t>();
        case type_t::SYMBOL:
            ret[0] = token.get_value<symbol_t>();
            return ret;
        case type_t::STRING_CONSTANT:
            return token.get_value<string_constant_t>();
        case type_t::INT_CONSTANT:
            return std::to_string(token.get_value<int_constant_t>());
    }
}
