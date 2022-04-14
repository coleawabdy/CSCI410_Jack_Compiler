#include "compiler.hpp"
#include "error.hpp"

#include <iostream>

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cerr << "A source path must be provided" << std::endl;
        return 1;
    } else if(argc > 2) {
        std::cerr << "Only 1 argument allowed" << std::endl;
    }

    compiler compile;

    try {
        compile(argv[1]);
    } catch(const io_error& e) {
        std::cerr << "I/O Error (" << e.get_filename().stem().string() << "): " << e.what();
    }

    return 0;
}