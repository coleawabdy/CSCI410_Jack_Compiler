#include "compiler.hpp"
#include "error.hpp"
#include "tokenizer.hpp"

#include <iostream>
#include <future>
#include <list>

void compiler::compile(const std::filesystem::path &source_path) {
    try {
        if(!std::filesystem::exists(source_path))
            throw io_error("File/directory does not exist", source_path);

        auto path = std::filesystem::canonical(source_path);
        /*std::filesystem::path output_path = path;
        output_path += (--output_path.end())->string();
        output_path.replace_extension(".vm");
        _output.open(output_path);*/

        std::list<std::future<void>> futures;
        for(const auto& entry : std::filesystem::directory_iterator(path)) {
            if(entry.is_regular_file() && entry.path().extension() == SOURCE_FILE_EXTENSION)
                futures.push_back(std::async(std::launch::async, &compiler::_process_source_file, this, entry.path()));
        }

        // Resolve all futures
        for(auto& future : futures)
            future.get();

    } catch(const std::filesystem::filesystem_error& e) {
        throw io_error(e.what(), e.path1());
    }
}

void compiler::_process_source_file(const std::filesystem::path &source_file_path) {
    std::string class_name = source_file_path.stem().string();

    /*
     * Code segment for reading entire file into string obtained from StackOverflow
     * https://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c
     * Solution by Maik Beckmann
     */

    std::ifstream source_file_stream(source_file_path);
    std::string source_code( (std::istreambuf_iterator<char>(source_file_stream)),
                         (std::istreambuf_iterator<char>()));

    tokenizer tokenize;
    tokenize(source_code);

    std::filesystem::path token_file_path = class_name + "T";
    token_file_path.replace_extension(".xml");
    std::fstream token_stream(token_file_path, std::ios::out);
    token_stream << tokenize.to_string();
    token_stream.close();
}

void compiler::_write_output(const std::list<std::string> &output, const std::string &out_file_name) {
    std::ofstream out_stream(out_file_name);

    for(const auto& line : output)
        out_stream << line << std::endl;
}