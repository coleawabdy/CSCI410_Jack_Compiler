#pragma once

#include "token.hpp"

#include <string>
#include <list>
#include <regex>

class tokenizer {
private:
    std::list<token> _tokens;
public:
    tokenizer() = default;
    ~tokenizer() = default;

    void tokenize(std::string& source_code);

    inline void operator()(std::string& source_code) { tokenize(source_code); };

    std::string to_string();
private:
    static void _token_to_tag(const token &token, std::string &name, std::string &value);
    static void _token_keyword_to_string(const token::keyword_t&, std::string& str);
    static void _token_process_symbol(const token::symbol_t&, std::string& str);

    static void _source_remove_comments(std::string& source_code);
    static bool _source_next_token(token &token, std::string &source_code);
    static std::string _check_matches(const std::smatch &matches, std::string &source_code);
};