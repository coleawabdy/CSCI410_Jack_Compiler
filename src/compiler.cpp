#include "compiler.hpp"
#include "tokenizer.hpp"

#if N2T_COMPLIANT == 10
    #include <tinyxml2.h>
#endif

#include <iostream>
#include <future>
#include <list>

void compiler::run(std::filesystem::path source_path) {
    if(!std::filesystem::exists(source_path))
        throw error(source_path.string() + " does not exist");

    source_path = std::filesystem::canonical(source_path);

    std::list<std::filesystem::path> source_files;
    _scan_source_path(source_path, source_files);

    for(const auto& file : source_files) {
        auto ctx = new context();

        auto output_file = file;
        output_file.replace_extension(OUTPUT_FILE_EXTENSION);
        output_file = output_file.filename();

        ctx->source_path = file;
        ctx->output_path = source_path / output_file;

        _contexts.push_back(ctx);
    }

    generator::reset();

    std::vector<std::future<void>> futures;
    futures.clear();
    futures.reserve(source_files.size());
    for(context* ctx : _contexts)
        futures.push_back(std::async(std::launch::async, &compiler::_compile, ctx));

    std::list<std::string> errors;
    for(unsigned int i = 0; i < futures.size(); i++) {
        try {
            futures[i].get();
            delete _contexts[i];
        } catch(const std::runtime_error& e) {
            auto file_name = std::filesystem::relative(_contexts[i]->source_path, source_path);
            errors.emplace_back(std::string("[") + file_name.generic_string() + "]: " + e.what());
        }
    }

    if(!errors.empty())
        throw error(errors);
}

void compiler::_scan_source_path(std::filesystem::path &source_path, std::list<std::filesystem::path>& source_files) {
    if(std::filesystem::is_directory(source_path)) {
        for(const auto& entry : std::filesystem::directory_iterator(source_path)) {
            if(entry.is_regular_file() && entry.path().extension() == SOURCE_FILE_EXTENSION)
                source_files.emplace_back(entry.path());
        }
    } else if(std::filesystem::is_regular_file(source_path) && source_path.extension() == SOURCE_FILE_EXTENSION) {
        source_files.emplace_back(source_path);
        source_path.remove_filename();
    }
}

void compiler::_compile(compiler::context *ctx) {
    std::ifstream source_file_stream(ctx->source_path);

    if(source_file_stream.fail())
        throw std::runtime_error("Failed to open file");

    std::string source_code((std::istreambuf_iterator<char>(source_file_stream)),(std::istreambuf_iterator<char>()));

    ctx->tokenizer.run(source_code);
    ctx->lexer.run(ctx->tokenizer);
    ctx->generator.run(ctx->lexer.get_class());

    std::ofstream output_file(ctx->output_path);

    for(const auto& line : ctx->generator.get_vm_code())
        output_file << line << std::endl;

    output_file.close();
}


