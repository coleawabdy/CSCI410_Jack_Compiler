#include "tokenizer.hpp"

#include <sstream>
#include <regex>
#include <iostream>
#include <unordered_map>

void tokenizer::reset() {
    _tokens_it = _tokens.cbegin();
}

const token& tokenizer::next() {
    return *(_tokens_it++);
}

const token& tokenizer::peek(uint32_t offset) {
    auto tk = _tokens_it;
    for(auto i = 0; i < offset; i++)
        tk++;
    return *tk;
}

bool tokenizer::has_next() {
    return _tokens_it != _tokens.cend();
}

void tokenizer::run(std::string &source_code) {
    _tokens.clear();

    _source_remove_comments(source_code);

    token tk;
    while(_source_next_token(tk, source_code))
        _tokens.emplace_back(tk);
}

void tokenizer::_source_remove_comments(std::string &source_code) {
    // Multi line comments
    auto comment_multiline_ex = std::regex(R"(\/\*(\*(?!\/)|[^*])*\*\/)", std::regex_constants::ECMAScript);
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
    if(std::isalnum(source_code[0])) {
        source_code.insert(source_code.begin(), match.begin(), match.end());
    } else if(!match.empty()) {
        token.type = token::type_t::KEYWORD;
        token.value = token::keyword_from_string(match);
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
        case '&':
            str = "&amp;";
            break;
        default:
            str = symbol;
            break;
    }
}