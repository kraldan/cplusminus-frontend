/**
 * This program tests that a sample can be compiled all the way to the executable,
 * and produces correct output and return code.
 */
#include <boost/process.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "ll_builder/LLBuilder.h"
#include "parser/Parser.h"
#include "semantic_checker/SemanticChecker.h"
#include "tests/configure.cmake.h"
#include "type/TypeManager.h"

using namespace std::string_literals;

namespace {
    struct ProcessResult {
        int exit_code;
        std::string out;
        std::string err;
    };

    std::ostream &operator<<(std::ostream &os, const ProcessResult &obj) {
        return os << "Process ended with exit code " << obj.exit_code <<
                  std::endl << " stdout='" << obj.out <<
                  std::endl << " stderr='" << obj.err << "'";
    }

    std::string streamContentToString(boost::process::ipstream &stream) {
        std::istreambuf_iterator<char> begin(stream), end;
        return {begin, end};
    }

    std::optional<std::string> readFile(const std::filesystem::path &path) {
        if (!std::filesystem::exists(path)) {
            return {};
        }

        std::ifstream ifs(path);
        std::ostringstream ss;
        ss << ifs.rdbuf();
        return ss.str();
    }
}

ProcessResult runProcess(const std::string &executable, std::initializer_list<std::string> args,
                         const std::optional<std::string> &input) {
    namespace bp = boost::process;
    bp::pipe stdinPipe;
    bp::ipstream stdoutStream;
    bp::ipstream stderrStream;

    bp::child compileProcess(
            executable,
            bp::args = args,
            bp::std_in < stdinPipe,
            bp::std_out > stdoutStream,
            bp::std_err > stderrStream);

    if (input) {
        stdinPipe.write(input->data(), input->size());
    }
    stdinPipe.close();

    compileProcess.wait();

    return {compileProcess.exit_code(), streamContentToString(stdoutStream),
            streamContentToString(stderrStream)};
}

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
    cpm::sc::SemanticChecker semanticChecker(context, std::cout);
    cpm::LLBuilder llBuilder;

    std::ostringstream llvmIRStream;
    try {
        ast = p.parse();
        semanticChecker.run(*ast);
        llBuilder.run(ast.get());
        llBuilder.dumpModule(llvmIRStream);
    } catch (const std::exception &e) {
        std::cout << "error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    const auto inputFilepath = std::filesystem::path{argv[1]};

    const auto testdir = std::filesystem::path{CMAKE_CURRENT_BINARY_DIR} / "tests" / "run" /
                         inputFilepath.filename();
    const auto executable = testdir / "a.out";
    const auto tmpOutput = testdir / "output.bin";

    const auto fileExitCode =
            inputFilepath.parent_path() / (std::string{inputFilepath.stem()} + ".ret"s);
    const auto fileRunInput =
            inputFilepath.parent_path() / (std::string{inputFilepath.stem()} + ".in"s);
    const auto fileRunOutput =
            inputFilepath.parent_path() / (std::string{inputFilepath.stem()} + ".output"s);
    const auto fileDontRun =
            inputFilepath.parent_path() / (std::string{inputFilepath.stem()} + ".dontrun"s);

    if (std::filesystem::exists(fileDontRun)) {
        std::cout << ".dontrun found" << std::endl;
        return EXIT_SUCCESS;
    }

    std::filesystem::remove_all(testdir);
    std::filesystem::create_directories(testdir);

    auto compilerInput = llvmIRStream.str();
    auto compiler = runProcess(CLANG_EXECUTABLE,
                               {"-x", "ir", "-", "-Wno-override-module", "-o", executable},
                               compilerInput);
    if (compiler.exit_code != 0) {
        std::cout << "Compile: " << compiler << std::endl;
        return EXIT_FAILURE;
    }

    auto runInput = readFile(fileRunInput);
    auto run = runProcess(executable, {}, readFile(fileRunInput));

    auto returnValue = readFile(fileExitCode).value_or("0");
    if (run.exit_code != std::stoi(returnValue)) {
        std::cout << "Run expected exit code = " << returnValue << std::endl;
        std::cout << "Run: " << run << std::endl;
        return EXIT_FAILURE;
    }

    auto expectedOutput = readFile(fileRunOutput);
    if (expectedOutput && run.out != expectedOutput) {
        std::cout << "Run output mismatch. Expected stdout = " << *expectedOutput << std::endl;
        std::cout << "Run: " << run << std::endl;
        std::cout << "Output saved as file " << tmpOutput << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
