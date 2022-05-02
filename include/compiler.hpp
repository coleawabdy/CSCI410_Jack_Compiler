#pragma once

#include "tokenizer.hpp"
#include "lexer.hpp"
#include "generator.hpp"

#include <filesystem>
#include <mutex>
#include <fstream>

class compiler {
public:
    class error : public std::exception {
    private:
        std::list<std::string> _errors;
        std::string _message;
    public:
        explicit error(std::string message) : _message(std::move(message)) {};
        explicit error(std::list<std::string> errors) : _errors(std::move(errors)) {};
        error() = default;

        [[nodiscard]] const char* what() const noexcept final {
            std::stringstream out;

            if(!_message.empty())
                out << _message << std::endl;

            if(!_errors.empty()) {
                out << _errors.size() << " error(s) reported:" << std::endl;
                for(const auto& error : _errors)
                    out << "\t" << error << std::endl;
            }

            auto out_str = out.str();
            out_str.shrink_to_fit();

            char* ret = new char[out_str.size() + 1];
            memcpy(ret, out_str.data(), out_str.size());
            ret[out_str.size()] = '\0';

            return ret;
        }
    };

    const std::string SOURCE_FILE_EXTENSION = ".jack";
    const std::string OUTPUT_FILE_EXTENSION = ".vm";
private:
    struct context {
        std::filesystem::path source_path;
        std::filesystem::path output_path;
        tokenizer tokenizer;
        lexer lexer;
        generator generator;
    };

    std::vector<context*> _contexts;
public:
    compiler() = default;
    ~compiler() = default;

    void run(std::filesystem::path source_path);
private:
    void _scan_source_path(std::filesystem::path &source_path, std::list<std::filesystem::path>& source_files);

    static void _compile(context* ctx);
};