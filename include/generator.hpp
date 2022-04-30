#pragma once

#include <list>
#include <string>


class generator {
private:
    std::list<std::string> _vm_code;
public:
    generator() = default;
    ~generator() = default;

    [[nodiscard]] const std::list<std::string>& get_vm_code() const { return _vm_code; };
};