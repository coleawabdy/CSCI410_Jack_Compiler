#pragma once

#include "token.hpp"

#include <string>
#include <list>
#include <regex>

class tokenizer {
private:
    std::list<token> _tokens;
    std::list<token>::const_iterator _tokens_it;
public:
    tokenizer() = default;
    ~tokenizer() = default;

    void run(std::string& source_code);

    void reset();
    const token& next();
    const token& peek(uint32_t offset = 0);
    bool has_next();
private:
    static void _token_process_symbol(const token::symbol_t&, std::string& str);

    static void _source_remove_comments(std::string& source_code);
    static bool _source_next_token(token &token, std::string &source_code);
    static std::string _check_matches(const std::smatch &matches, std::string &source_code);
};