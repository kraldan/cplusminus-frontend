#pragma once

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents a continue statement.
     */
    class ContinueStmt : public Node {
    public:
        ContinueStmt(SourceInfo src_info, size_t n_loops) :
                Node(std::move(src_info)),
                continue_level(n_loops) {}

        // which loop should we continue in?
        size_t continue_level;
    };
}

