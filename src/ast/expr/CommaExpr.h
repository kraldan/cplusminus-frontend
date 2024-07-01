#pragma once

#include <vector>

#include "expr.h"
#include "ast/base/node_ptr.h"

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents a comma expression.
     *
     * Example:
     * 'i = 5, c = 'a', f(), a[5];'
     */
    class CommaExpr : public Node {
    public:
        CommaExpr(SourceInfo src_info, std::vector<node_ptr<Expr>> exprs);

        // clang issues with vector
        CommaExpr(CommaExpr &&) = default;

        std::vector<node_ptr<Expr>> expressions;
    };
}
