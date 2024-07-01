#pragma once

#include <vector>

#include "ast/base/Node.h"
#include "ast/expr/expr.h"

namespace ast {
    /**
     * Represents an implicit default argument added during a function call.
     *
     * Example:
     * '
     * int foo(int a = 5);
     * int a = foo()
     * '
     * During semantic analysis, DefaultArgExpr node will be added
     * to the arg list of 'foo()'.
     *
     * Use --ast-dump for an example.
     */
    class DefaultArgExpr : public Node {
    public:
        DefaultArgExpr(SourceInfo src_info, Expr *expr);

        /**
         * This is only a pointer and not a node_ptr, because the object
         * is owned by another ast node (an ast::Param).
         */
        Expr *expr;
    };
}
