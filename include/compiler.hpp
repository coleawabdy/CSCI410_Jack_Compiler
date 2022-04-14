#pragma once

#include <filesystem>
#include <queue>
#include <vector>
#include <fstream>
#include <mutex>
#include <list>

class compiler {
public:
    const std::string SOURCE_FILE_EXTENSION = ".jack";
private:
    std::ofstream _output;
    std::mutex _output_mutex;
public:
    compiler()  = default;
    ~compiler() = default;

    void compile(const std::filesystem::path& source_path);

    inline void operator()(const std::filesystem::path& source_path) { compile(source_path); };

private:
    void _process_source_file(const std::filesystem::path& source_file_path);
    void _write_output(const std::list<std::string>& output, const std::string& out_file_name);
};