/**
 * This program tests that AST dumping works on a sample without errors.
 */
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include "ast_dumper/AstDumper.h"
#include "parser/Parser.h"
#include "semantic_checker/SemanticChecker.h"

using namespace std::string_literals;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Missing filepath" << std::endl;
        return EXIT_FAILURE;
    }

    const auto inputFilepath = std::filesystem::path{argv[1]};
    const auto inputAstPath =
            inputFilepath.parent_path() / (std::string{inputFilepath.stem()} + ".ast"s);
    const auto fileDontRun =
            inputFilepath.parent_path() / (std::string{inputFilepath.stem()} + ".dontrun"s);

    if (std::filesystem::exists(fileDontRun)) {
        std::cout << ".dontrun found" << std::endl;
        return EXIT_SUCCESS;
    }

    std::ifstream ifs(inputFilepath);
    if (!ifs || !ifs.is_open()) {
        std::cout << "File " << argv[1] << " could not be opened." << std::endl;
        return EXIT_FAILURE;
    }
    std::ostringstream astExpected;
    {
        std::ifstream ifsAst(inputAstPath);
        if (!ifsAst || !ifsAst.is_open()) {
            std::cout << "File " << inputAstPath << " could not be opened." << std::endl;
            return EXIT_FAILURE;
        }
        astExpected << ifsAst.rdbuf();
    }

    cpm::Context context(ifs);
    Parser p(context);
    ast::node_ptr<ast::TranslationUnit> ast;
    cpm::sc::SemanticChecker semanticChecker(context, std::cout);
    AstDumper astDumper;

    std::ostringstream astGenerated;
    try {
        ast = p.parse();
        astDumper.run(*ast, std::cout);    // check ony that --ast-dump-raw works
        semanticChecker.run(*ast);
        astDumper.run(*ast, astGenerated);      // --ast-dump into ostream
    } catch (const std::exception &e) {
        std::cout << "error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    // note: running 'diff' on two files might be faster than string comparison,
    //       I just couldn't get it to work with boost
    return astGenerated.str() == astExpected.str() ? EXIT_SUCCESS : EXIT_FAILURE;
}
