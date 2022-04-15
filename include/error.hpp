#pragma once

#include <stdexcept>
#include <filesystem>
#include <utility>

class io_error : public std::runtime_error {
private:
    const std::filesystem::path _filename;
public:
    inline io_error(const std::string& message, std::filesystem::path filename = "") :
    std::runtime_error(message),
    _filename(std::move(filename)) {};

    [[nodiscard]] inline const std::filesystem::path& get_filename() const noexcept { return _filename; };
};

class tokenizing_error : public std::runtime_error {
private:
    const std::filesystem::path _filename;
public:
    inline tokenizing_error(const std::string& message) :
            std::runtime_error(message) {};
};
