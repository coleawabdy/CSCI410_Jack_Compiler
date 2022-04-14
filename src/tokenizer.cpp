#include "tokenizer.hpp"

#include <sstream>

void tokenizer::tokenize(std::string &source_code) {
    _tokens.clear();
}

std::string tokenizer::to_string() {
    std::stringstream out;

    out << "<tokens>" << std::endl;

    std::string tag_name, tag_value;
    for(auto const& token : _tokens) {
        switch(token.type) {
            case token::type_t::KEYWORD:
                tag_name = "keyword";
                break;
            case token::type_t::IDENTIFIER:
                tag_name = "identifier";
                break;
            case token::type_t::SYMBOL:
                tag_name = "symbol";
                break;
            case token::type_t::STRING_CONSTANT:
                tag_name = "stringConstant";
                break;
            case token::type_t::INT_CONSTANT:
                tag_name = "integerConstant";
                break;
        }

        out
            << "<"
            << tag_name
            << "> "
            << tag_value
            << " </"
            << tag_name
            << ">"
            << std::endl;
    }

    out << "</tokens>" << std::endl << std::endl;

    return out.str();
}