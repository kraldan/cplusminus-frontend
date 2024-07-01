#pragma once

#include <stdexcept>
#include <string>

#include "ast/base/SourceInfo.h"

namespace cpm {

    class CompilationError : std::runtime_error {
    public:
        CompilationError(const std::string &msg, ast::SourceInfo src_info);

        /**
         * Get only the string message without source information.
         */
        const std::string &msg();

        const ast::SourceInfo &src_info();

    private:
        const std::string _msg;
        const ast::SourceInfo _src_info;

        /**
         * Generate 'what' string for std::runtime_error.
         */
        static std::string generate_what(const std::string &msg,
                                         ast::SourceInfo src_info);
    };
}
