#include "CompilationError.h"

using namespace std::string_literals;
using namespace std;

namespace cpm {
    CompilationError::CompilationError(const std::string &msg, ast::SourceInfo src_info) :
            std::runtime_error(generate_what(msg, src_info)),
            _msg(msg),
            _src_info(src_info) {}

    const std::string &CompilationError::msg() {
        return _msg;
    }

    const ast::SourceInfo &CompilationError::src_info() {
        return _src_info;
    }

    std::string CompilationError::generate_what(const std::string &msg,
                                                ast::SourceInfo src_info) {
        std::string loc = std::to_string(src_info.line_no) + ":"s + std::to_string(src_info.col_no);
        return loc + ": error: " + msg;
    }
}