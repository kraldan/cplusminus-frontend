#include <iostream>
#include <fstream>
#include <optional>

#include <boost/program_options.hpp>

#include "parser/Parser.h"
#include "semantic_checker/SemanticChecker.h"
#include "ll_builder/LLBuilder.h"
#include "ast_dumper/AstDumper.h"

enum class ReturnValue : int32_t {
    Success = EXIT_SUCCESS,
    Failure = EXIT_FAILURE,
    FileNotFound = 2,
    FileOpen = 2,
    AntlrSyntaxError = 3,
    AntlrVisitError = 4,
    ScVisitError = 5,
};

int exitCode(ReturnValue ret) {
    return static_cast<std::underlying_type_t<ReturnValue>>(ret);
}

using namespace std;
using namespace ast;

namespace po = boost::program_options;

std::ostream &file_or_cout(optional<ofstream> &output_file) {
    if (!output_file) {
        return std::cout;
    }

    return *output_file;
}

int main(int argc, char **argv) {
    optional<ofstream> output_file;

    po::options_description generic("Allowed options");
    generic.add_options()
            ("help,h", "produce help message")
            ("output,o", po::value<string>(), "output file")
            ("ast-dump-raw", "dump AST before semantic analysis")
            ("ast-dump", "dump AST after semantic analysis")
            ("ir", "output llvm ir (default)")
            ("input-file", "input file (option can be omitted)");

    po::positional_options_description p;
    p.add("input-file", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
            options(generic).positional(p).run(), vm);
    vm.notify();

    // ----------------------------------------------------------------

    if (vm.count("help")) {
        cout << generic << endl;
        return exitCode(ReturnValue::Success);
    }

    if (!vm.count("input-file")) {
        cout << "missing input file" << endl;
        return exitCode(ReturnValue::FileNotFound);
    }

    ifstream file(vm["input-file"].as<string>());
    if (!file) {
        cout << "couldn't open file: " << vm["input-file"].as<string>() << endl;
        return exitCode(ReturnValue::FileOpen);
    }

    cpm::Context context(file);
    file.close();

    Parser parser(context);
    cpm::sc::SemanticChecker semantic_checker(context, std::cout);
    AstDumper ast_dumper;
    cpm::LLBuilder ll_builder;
    ast::node_ptr<ast::TranslationUnit> ast;

    try {
        ast = parser.parse();
    }
    catch (const Parser::SyntaxError &e) {
        std::cerr << "error: " << e.what() << std::endl;
        return exitCode(ReturnValue::AntlrSyntaxError);
    }
    catch (const Parser::VisitError &e) {
        std::cerr << "error: " << e.what() << std::endl;
        return exitCode(ReturnValue::AntlrVisitError);
    }

    if (vm.count("output")) {
        output_file = ofstream(vm["output"].as<string>());
        if (!*output_file) {
            std::cerr << "couldn't open file: " << vm["output"].as<string>() << endl;
            return exitCode(ReturnValue::FileOpen);
        }
    }

    //--------- dump raw ast is the user chooses ------------
    if (vm.count("ast-dump-raw")) {
        ast_dumper.run(*ast, file_or_cout(output_file));
        return exitCode(ReturnValue::Success);
    }

    //--------------- perform semantic analysis ----------------
    try {
        semantic_checker.run(*ast);
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << endl;
        return exitCode(ReturnValue::ScVisitError);
    }

    //------------- dump ast if the user chooses -------------
    if (vm.contains("ast-dump")) {
        ast_dumper.run(*ast, file_or_cout(output_file));
        return exitCode(ReturnValue::Success);
    }

    //------------- generate llvm ir ----------------------
    ll_builder.run(ast.get());
    ll_builder.dumpModule(file_or_cout(output_file));
    if (ll_builder.verifyModule()) {
        cout << "LLVM module verification failed" << endl;
        return exitCode(ReturnValue::Failure);
    }
    return exitCode(ReturnValue::Success);
}
