/**
 * This program tests that a sample fails during parsing.
 */
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "parser/Parser.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Missing filepath" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream ifs(argv[1]);
    if (!ifs || !ifs.is_open()) {
        std::cout << "File " << argv[1] << " could not be opened." << std::endl;
        return EXIT_FAILURE;
    }

    cpm::Context context(ifs);
    Parser p(context);

    try {
        p.parse();
        std::cout << "error: file parsed though it should fail" << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception &e) {
        return EXIT_SUCCESS;
    }
}
