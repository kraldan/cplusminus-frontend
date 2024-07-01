#pragma once

#include <utility>

#include "ast/base/node_ptr.h"
#include "ast/base/Node.h"
#include "ast/expr/expr.h"
#include "type/Type.h"


namespace ast {

    /**
     * Represents an implicit type cast (conversion).
     *
     * This includes integer promotions: they are treated
     * the same as other type conversions.
     *
     * Example:
     * 'double d = 4'
     * the 4 undergoes an implicit 'int->double' conversion.
     */
    class ImplicitTypeCastExpr : public Node {
    public:
        ImplicitTypeCastExpr(SourceInfo src_info, node_ptr<Expr> val, cpm::Type * dest_ty);

        node_ptr<Expr> val;
        cpm::Type * dest_ty;
    };

}