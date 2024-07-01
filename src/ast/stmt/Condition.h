#pragma once

#include "ast/expr/expr.h"
#include "ast/base/node_ptr.h"

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents a condition in the program, such as
     * in an IfStmt or ForStmt.
    */
    class Condition : public Node {
    public:
        explicit Condition(SourceInfo src_info, node_ptr<Expr> expr) :
		Node(std::move(src_info)),
        expr(std::move(expr))
        {}

        node_ptr<Expr> expr;
    };
}
