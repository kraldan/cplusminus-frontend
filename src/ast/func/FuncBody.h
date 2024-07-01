#pragma once

#include "ast/base/node_ptr.h"
#include "ast/stmt/CompoundStmt.h"

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents a function body.
     *
     * Example:
     * '
     * int foo() {
     *      return 42;
     * }
     * '
     * the
     * ' {
     * return 42;
     * }
     * ' is the function body.
     */
    class FuncBody : public Node {
    public:
        explicit FuncBody(SourceInfo src_info, node_ptr<CompoundStmt> comp_stmt) :
                Node(std::move(src_info)),
                comp_stmt(std::move(comp_stmt)) {}

        node_ptr<CompoundStmt> comp_stmt;
        // in the future, memory initializer might be added for constructors
    };
}
