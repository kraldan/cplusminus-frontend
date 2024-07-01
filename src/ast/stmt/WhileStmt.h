#pragma once

#include "ast/base/node_ptr.h"
#include "Condition.h"
#include "stmt.h"

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents a while statement.
     */
    class WhileStmt : public Node {
    public:
        WhileStmt(SourceInfo src_info, node_ptr<Condition> cond, node_ptr<Stmt> body) :
        Node(std::move(src_info)),
        cond(std::move(cond)),
        body(std::move(body))
        {}

        node_ptr<Condition> cond;
        node_ptr<Stmt> body;
    };
}

