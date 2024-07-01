#pragma once

#include "ast/base/Node.h"
#include "ast/expr/expr.h"


namespace ast {

    /**
     * Represents implicit array to pointer decay conversion.
     */
    class ArrToPtrExpr : public Node {
    public:
        explicit ArrToPtrExpr(SourceInfo src_info, node_ptr<Expr> arr_expr);

        // the expression that has array type
        node_ptr<Expr> arr_expr;
    };

}