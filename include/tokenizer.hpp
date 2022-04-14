#pragma once

#include "token.hpp"

#include <string>
#include <list>

class tokenizer {
private:
    std::list<token> _tokens;
public:
    tokenizer() = default;
    ~tokenizer() = default;

    void tokenize(std::string& source_code);

    inline void operator()(std::string& source_code) { tokenize(source_code); };

    std::string to_string();
};