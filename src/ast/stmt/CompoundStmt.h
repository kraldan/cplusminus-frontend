#pragma once

#include <vector>

#include "stmt.h"
#include "ast/base/node_ptr.h"

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents a compound statement,
     * e.g. a series of statements inside curly braces.
     */
    class CompoundStmt : public Node {
    public:
        explicit CompoundStmt(SourceInfo src_info,
                              std::vector<node_ptr<Stmt>> stats) :
                Node(std::move(src_info)),
                statements(std::move(stats)) {}

        // clang issues with vector
        CompoundStmt(CompoundStmt &&) = default;

        std::vector<node_ptr<Stmt>> statements;
    };
}
