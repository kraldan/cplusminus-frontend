#pragma once

#include "ast/base/node_ptr.h"
#include "expr.h"

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents conditional ternary expression.
     *
     * Example:
     * 'a ? b : c'
     */
    class TernaryExpr : public Node {
    public:
        TernaryExpr(SourceInfo src_info, node_ptr<Expr> cond, node_ptr<Expr> then, node_ptr<Expr> else_);

        node_ptr<Expr> cond;
        node_ptr<Expr> then;
        node_ptr<Expr> else_;
    };
}

