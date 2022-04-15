#include "tokenizer.hpp"
#include "error.hpp"

#include <sstream>
#include <regex>
#include <iostream>
#include <unordered_map>

const std::unordered_map<std::string, token::keyword_t> KEYWORD_DECODER = {
        {"class",       token::keyword_t::CLASS},
        {"method",      token::keyword_t::METHOD},
        {"function",    token::keyword_t::FUNCTION},
        {"constructor", token::keyword_t::CONSTRUCTOR},
        {"int",         token::keyword_t::INT},
        {"bool",        token::keyword_t::BOOL},
        {"char",        token::keyword_t::CHAR},
        {"void",        token::keyword_t::VOID},
        {"var",         token::keyword_t::VAR},
        {"static",      token::keyword_t::STATIC},
        {"field",       token::keyword_t::FIELD},
        {"let",         token::keyword_t::LET},
        {"do",          token::keyword_t::DO},
        {"if",          token::keyword_t::IF},
        {"else",        token::keyword_t::ELSE},
        {"while",       token::keyword_t::WHILE},
        {"return",      token::keyword_t::RETURN},
        {"true",        token::keyword_t::TRUE},
        {"false",       token::keyword_t::FALSE},
        {"null",        token::keyword_t::NUL},
        {"this",        token::keyword_t::THIS}
};

void tokenizer::tokenize(std::string &source_code) {
    _tokens.clear();

    _source_remove_comments(source_code);

    token tk;
    while(_source_next_token(tk, source_code))
        _tokens.emplace_back(tk);
}

std::string tokenizer::to_string() {
    std::stringstream out;

    out << "<tokens>" << std::endl;

    std::string tag_name, tag_value;
    for(auto const& token : _tokens) {
        _token_to_tag(token, tag_name, tag_value);

        out
            << "<"
            << tag_name
            << "> "
            << tag_value.c_str()
            << " </"
            << tag_name
            << ">"
            << std::endl;
    }

    out << "</tokens>" << std::endl;

    return out.str();
}

void tokenizer::_token_to_tag(const token &token, std::string &name, std::string &value) {
    switch(token.type) {
        case token::type_t::KEYWORD:
            name = "keyword";
            _token_keyword_to_string(token.get_value<token::keyword_t>(), value);
            break;
        case token::type_t::IDENTIFIER:
            name = "identifier";
            value = token.get_value<token::identifier_t>();
            break;
        case token::type_t::SYMBOL:
            name = "symbol";
            _token_process_symbol( token.get_value<token::symbol_t>(), value);
            break;
        case token::type_t::STRING_CONSTANT:
            name = "stringConstant";
            value = token.get_value<token::string_constant_t>();
            break;
        case token::type_t::INT_CONSTANT:
            name = "integerConstant";
            value = std::to_string(token.get_value<token::int_constant_t>());
            break;
    }
}

void tokenizer::_token_keyword_to_string(const token::keyword_t &keyword, std::string &str) {
    switch (keyword) {
        case token::keyword_t::CLASS:
            str = "class";
            break;
        case token::keyword_t::METHOD:
            str = "method";
            break;
        case token::keyword_t::FUNCTION:
            str = "function";
            break;
        case token::keyword_t::CONSTRUCTOR:
            str = "constructor";
            break;
        case token::keyword_t::INT:
            str = "int";
            break;
        case token::keyword_t::BOOL:
            str = "bool";
            break;
        case token::keyword_t::CHAR:
            str = "char";
            break;
        case token::keyword_t::VOID:
            str = "void";
            break;
        case token::keyword_t::VAR:
            str = "var";
            break;
        case token::keyword_t::STATIC:
            str = "static";
            break;
        case token::keyword_t::FIELD:
            str = "field";
            break;
        case token::keyword_t::LET:
            str = "let";
            break;
        case token::keyword_t::DO:
            str = "do";
            break;
        case token::keyword_t::IF:
            str = "if";
            break;
        case token::keyword_t::ELSE:
            str = "else";
            break;
        case token::keyword_t::WHILE:
            str = "while";
            break;
        case token::keyword_t::RETURN:
            str = "return";
            break;
        case token::keyword_t::TRUE:
            str = "true";
            break;
        case token::keyword_t::FALSE:
            str = "false";
            break;
        case token::keyword_t::NUL:
            str = "null";
            break;
        case token::keyword_t::THIS:
            str = "this";
            break;
    }
}

void tokenizer::_source_remove_comments(std::string &source_code) {
    // Multi line comments
    auto comment_multiline_ex = std::regex(R"((\/\*.*?\*\/)+)", std::regex_constants::ECMAScript);
    source_code = std::regex_replace(source_code, comment_multiline_ex, "");

    // Single line comments
    auto comment_ex = std::regex(R"((\/\/)(.+?)([\n\r]|\*\)))", std::regex_constants::ECMAScript);
    source_code = std::regex_replace(source_code, comment_ex, "");

    // Remove duplicate newlines
    auto newline_ex = std::regex("(\n|\r){1,}", std::regex_constants::ECMAScript);
    source_code = std::regex_replace(source_code, newline_ex, "\n");
}

bool tokenizer::_source_next_token(token &token, std::string &source_code) {
    const static auto REGEX_WHITESPACE  = std::regex(
            "(\\s|\r|\n)+",
            std::regex_constants::ECMAScript);
    const static auto REGEX_SYMBOL      = std::regex(
            R"(\{|\}|\(|\)|\[|\]|\.|,|;|\+|-|\*|\/|&|\||>|<|=|~)",
            std::regex_constants::ECMAScript);
    const static auto REGEX_KEYWORD     = std::regex(
            "class|constructor|function|method|field|static|var|int|char|boolean|void|true|false|null|this|let|do|if|else|while|return",
            std::regex_constants::ECMAScript);
    const static auto REGEX_INT_CONST   = std::regex(
            "\\d+",
            std::regex_constants::ECMAScript);
    const static auto REGEX_STR_CONST   = std::regex(
            R"("[^"]*")",
            std::regex_constants::ECMAScript);
    const static auto REGEX_IDENTIFIER  = std::regex(
            "([a-zA-Z]|_)([a-zA-Z]|\\d|_)*",
            std::regex_constants::ECMAScript);

    source_code = std::regex_replace(source_code, REGEX_WHITESPACE, "", std::regex_constants::match_continuous);

    std::smatch matches;
    std::string match;

    std::regex_search(source_code, matches, REGEX_SYMBOL, std::regex_constants::match_continuous);
    match = _check_matches(matches, source_code);
    if(!match.empty()) {
        token.type = token::type_t::SYMBOL;
        token.value = token::symbol_t(match[0]);
        return true;
    }

    std::regex_search(source_code, matches, REGEX_KEYWORD, std::regex_constants::match_continuous);
    match = _check_matches(matches, source_code);
    if(!match.empty()) {
        token.type = token::type_t::KEYWORD;
        token.value = KEYWORD_DECODER.at(match);
        return true;
    }

    std::regex_search(source_code, matches, REGEX_INT_CONST, std::regex_constants::match_continuous);
    match = _check_matches(matches, source_code);
    if(!match.empty()) {
        token.type = token::type_t::INT_CONSTANT;
        token.value = token::int_constant_t(std::stoi(match));
        return true;
    }

    std::regex_search(source_code, matches, REGEX_STR_CONST, std::regex_constants::match_continuous);
    match = _check_matches(matches, source_code);
    if(!match.empty()) {
        token.type = token::type_t::STRING_CONSTANT;
        token.value = token::string_constant_t(match.substr(1, match.size() - 2));
        return true;
    }

    std::regex_search(source_code, matches, REGEX_IDENTIFIER, std::regex_constants::match_continuous);
    match = _check_matches(matches, source_code);
    if(!match.empty()) {
        token.type = token::type_t::IDENTIFIER;
        token.value = token::identifier_t(match);
        return true;
    }

    return false;
}

std::string tokenizer::_check_matches(const std::smatch &matches, std::string &source_code) {

    if(!matches.empty()) {
        std::string ret = matches[0].str();
        source_code.erase(matches[0].first, matches[0].second);
        return ret;
    }

    return "";
}

void tokenizer::_token_process_symbol(const token::symbol_t & symbol, std::string &str) {
    switch(symbol) {
        case '<':
            str = "&lt;";
            break;
        case '>':
            str = "&gt;";
            break;
        default:
            str = symbol;
            break;
    }
}
