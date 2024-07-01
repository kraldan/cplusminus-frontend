#pragma once

#include <variant>

#include "ast/base/node_ptr.h"
#include "expr.h"

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents call of the [] operator.
     *
     * Example:
     * 'a[5]'
     */
    class SubscriptExpr : public Node {
    public:
        SubscriptExpr(SourceInfo src_info, node_ptr<Expr> dest,
        node_ptr<Expr> index);

        node_ptr<Expr> dest;
        node_ptr<Expr> index;
    };
}

