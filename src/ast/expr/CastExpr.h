#pragma once

#include "ast/base/Node.h"
#include "ast/base/node_ptr.h"
#include "expr.h"
#include "type/Type.h"

namespace ast {
    /**
     * Represents an explicit (c-style) cast expression.
     *
     * Example:
     * '(int *) malloc(...)'
     */
    class CastExpr : public Node {
    public:
        CastExpr(SourceInfo src_info, cpm::Type *type, node_ptr<Expr> expr);

        cpm::Type *type;
        node_ptr<Expr> expr;
    };
}
