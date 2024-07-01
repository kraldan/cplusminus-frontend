#pragma once

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents a break statement.
     */
    class BreakStmt : public Node {
    public:
        explicit BreakStmt(SourceInfo src_info, size_t break_level) :
                Node(std::move(src_info)),
                break_level(break_level) {};

        // how many loops should we break from?
        size_t break_level;
    };
}
