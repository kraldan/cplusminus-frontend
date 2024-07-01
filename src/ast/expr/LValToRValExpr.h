#pragma once

#include "ast/base/Node.h"
#include "ast/base/node_ptr.h"
#include "expr.h"


namespace ast {

    /**
     * Represents an implicit lvalue to rvalue conversion.
     *
     * Example:
     * 'a + 5'
     * 'a' has to undergo lvalue-to-rvalue conversion before
     * the plus operation can be performed.
     */
    class LValToRValExpr : public Node {
    public:
        LValToRValExpr(SourceInfo src_info, node_ptr<Expr> val);

        node_ptr<Expr> val;
    };
}



