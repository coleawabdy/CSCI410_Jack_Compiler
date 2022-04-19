#include "compiler.hpp"
#include "tokenizer.hpp"

#if N2T_COMPLIANT == 10
    #include <tinyxml2.h>
#endif

#include <iostream>
#include <future>
#include <list>
#include <fstream>

void compiler::run(std::filesystem::path source_path) {
    if(!std::filesystem::exists(source_path))
        throw error(source_path.string() + " does not exist");

    source_path = std::filesystem::canonical(source_path);

    std::list<std::filesystem::path> source_files;
    _scan_source_path(source_path, source_files);

    for(const auto& file : source_files) {
        _contexts.push_back(new context());
        _contexts.back()->source_file = file;
    }

    std::vector<std::future<void>> futures;
    futures.clear();
    futures.reserve(source_files.size());
    for(context* ctx : _contexts)
        futures.push_back(std::async(std::launch::async, _compile, ctx));

    std::list<std::string> errors;
    for(unsigned int i = 0; i < futures.size(); i++) {
        try {
            futures[i].get();
            delete _contexts[i];
        } catch(const std::runtime_error& e) {
            auto file_name = std::filesystem::relative(_contexts[i]->source_file, source_path);
            errors.emplace_back(std::string("[") + file_name.generic_string() + "]: " + e.what());
        }
    }

    if(!errors.empty())
        throw error(errors);
}

void compiler::_scan_source_path(const std::filesystem::path &source_path, std::list<std::filesystem::path>& source_files) {
    for(const auto& entry : std::filesystem::directory_iterator(source_path)) {
        if(entry.is_regular_file() && entry.path().extension() == SOURCE_FILE_EXTENSION)
            source_files.emplace_back(entry.path());
#ifndef N2T_COMPLIANT
        if(entry.is_directory())
            _scan_source_path(entry.path(), source_files);
#endif
    }
}

void compiler::_compile(compiler::context *ctx) {
    std::ifstream source_file_stream(ctx->source_file);
    auto class_name = ctx->source_file.replace_extension("").filename();

    if(source_file_stream.fail())
        throw std::runtime_error("Failed to open file");

    std::string source_code((std::istreambuf_iterator<char>(source_file_stream)),(std::istreambuf_iterator<char>()));

    ctx->tokenizer.run(source_code);

#if N2T_COMPLIANT == 10
    ctx->tokenizer.reset();

    tinyxml2::XMLDocument doc;
    auto tokens = doc.NewElement("tokens");
    doc.LinkEndChild(tokens);
    while(ctx->tokenizer.has_next()) {
        auto token = ctx->tokenizer.next();
        tokens->InsertNewChildElement(token::type_to_string(token.type).c_str())->InsertNewText(token::to_string(token).c_str());
    }

    auto tokens_file_path = class_name.generic_string() + "T.xml";
    doc.SaveFile(tokens_file_path.c_str());
#endif

    ctx->lexer.run(ctx->tokenizer);

#if N2T_COMPLIANT == 10
    auto lexed_file_path = class_name.generic_string() + ".xml";
    std::ofstream lexed_xml_file;
    lexed_xml_file.open(lexed_file_path);
    tinyxml2::XMLPrinter printer(0, false, 0);
    ctx->lexer.get_document()->Print(&printer);
    lexed_xml_file << printer.CStr();
    lexed_xml_file.close();
#endif
}


