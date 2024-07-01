#pragma once

#include <memory>

#include "expr.h"
#include "operators.h"

#include "ast/base/Node.h"
#include "ast/base/node_ptr.h"

namespace ast {
    /**
     * Represents a binary expression.
     *
     * Examples:
     * 'a + 5'
     * 'a && b'
     */
    class BinaryExpr : public Node {
    public:
        BinaryExpr(SourceInfo src_info, node_ptr<Expr> left,
                   node_ptr<Expr> right,
                   BinaryOp op);

        node_ptr<Expr> lhs;
        node_ptr<Expr> rhs;
        BinaryOp op;
    };
}
