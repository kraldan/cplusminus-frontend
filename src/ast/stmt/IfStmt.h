#pragma once

#include <optional>

#include "ast/base/node_ptr.h"
#include "stmt.h"
#include "Condition.h"

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents an if statement with possible else statement.
     */
    class IfStmt : public Node {
    public:
        IfStmt(SourceInfo src_info, node_ptr<Condition> cond, node_ptr<Stmt> body, std::optional<node_ptr<Stmt>> else_body) :
		Node(std::move(src_info)),
        cond(std::move(cond)),
        body(std::move(body)),
        else_body(std::move(else_body))
        {}

        node_ptr<Condition> cond;
        node_ptr<Stmt> body;
        std::optional<node_ptr<Stmt>> else_body;
    };
}

