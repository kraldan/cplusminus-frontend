#pragma once

#include "ast/base/node_ptr.h"
#include "expr.h"
#include "ast/base/Node.h"

namespace ast {
    /** Represents the post-increment or post-decrement expression.
     *
     * Examples:
     * 'a++', 'b--'
     */
    class PostIncrExpr : public Node {
    public:
        PostIncrExpr(SourceInfo src_info, node_ptr<Expr> expr, bool incr);

        node_ptr<Expr> expr;
        // true for ++, false for --
        bool incr;
    };
}

