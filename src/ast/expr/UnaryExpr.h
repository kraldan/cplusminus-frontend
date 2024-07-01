#pragma once

#include "ast/base/Node.h"
#include "ast/base/node_ptr.h"
#include "operators.h"
#include "expr.h"

namespace ast {
    /**
     * Represents a unary expression.
     *
     * Examples:
     * '&a'
     * '*b'
     * '++c'
     */
    class UnaryExpr : public Node {
    public:
        UnaryExpr(SourceInfo src_info, UnaryOp op, node_ptr<Expr> expr);

        UnaryOp op;
        node_ptr<Expr> expr;
    };
}
