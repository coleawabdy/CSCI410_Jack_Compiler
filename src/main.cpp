#include "compiler.hpp"

#include <iostream>

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cerr << "A source path must be provided" << std::endl;
        return 1;
    } else if(argc > 2) {
        std::cerr << "Only 1 argument allowed" << std::endl;
    }

    compiler compiler;

    try {
        compiler.run(argv[1]);
    } catch(const compiler::error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}