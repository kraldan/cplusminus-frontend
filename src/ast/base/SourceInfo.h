#pragma once

#include <cstddef>
#include <string>

namespace ast {
    /**
     * Holds source code information.
     */
    struct SourceInfo {
        /**
         * Create a SourceInfo from line number and column number.
         */
        explicit SourceInfo(size_t line_no, size_t col_no);

        /**
         * Create a SourceInfo that represents unknown source location,
         * line 0:0.
         */
        explicit SourceInfo();

        std::string str() const {
            return std::to_string(line_no) + ":" + std::to_string(col_no);
        }

        // on which line in the file is the first token
        // of the node
        // line 0 represents unknown line or nodes added by the compiler
        // that don't reflect the original source code, such as implicitly
        // added function declarations
        size_t line_no;
        size_t col_no;

    private:
        // this member is here so that clang-tidy doesn't complain
        // when I use std::move on SourceInfo objects
        // -> for now the compiler is right, that moving is useless,
        // but in the future when this struct is extended, moving might
        // be needed/warranted
        std::string not_trivially_copyable_member;
    };
}
