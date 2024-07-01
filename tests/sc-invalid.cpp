/**
 * This program tests that a sample fails during semantic analysis.
 */
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "semantic_checker/SemanticChecker.h"
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
    ast::node_ptr<ast::TranslationUnit> ast;

    try {
        ast = p.parse();
    } catch (const std::exception &e) {
        std::cout << "error: file didn't pass parsing though it should" << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    cpm::sc::SemanticChecker semanticChecker(context, std::cout);
    try {
        semanticChecker.run(*ast);
        std::cout << "error: file passed semantic analysis though it should fail" << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception &e) {
        return EXIT_SUCCESS;
    }
}

