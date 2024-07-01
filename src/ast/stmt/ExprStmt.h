#pragma once

#include <optional>

#include "ast/expr/expr.h"
#include "ast/base/node_ptr.h"

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents an expression statement.
     *
     * For example:
     * void f() {
     *      foo();      // this is an expression statement
     * }
     */
    class ExprStmt : public Node {
    public:
        explicit ExprStmt(SourceInfo src_info, std::optional<node_ptr<Expr>> expr) :
		Node(std::move(src_info)),
        expr(std::move(expr))
        {}

        std::optional<node_ptr<Expr>> expr;
    };
}
