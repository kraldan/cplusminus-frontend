#pragma once

#include <optional>

#include "expr.h"

#include "type/Type.h"

#include "ast/base/Node.h"
#include "ast/base/node_ptr.h"
#include "operators.h"

namespace ast {
    /**
     * Represents assignment or compound assignment.
     *
     * Examples:
     * 'a = b'
     * 'a <<= 4'
     */
    class AssignmentExpr : public Node {
    public:
        AssignmentExpr(SourceInfo src_info, node_ptr<Expr> lhs,
                       node_ptr<Expr> rhs, AssignOp op);

        node_ptr<Expr> lhs;
        node_ptr<Expr> rhs;
        AssignOp op;

        /**
         * in case of compound assignment (e.g. '+='), the lhs value
         * might have to be converted to a different type (e.g. from int to double)
         * before the compute operation (e.g. '+') can be executed
         *
         * this should be set during semantic analysis
         */
        std::optional<cpm::Type *> lhs_type = std::nullopt;
    };
}

