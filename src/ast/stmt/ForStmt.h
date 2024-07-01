#pragma once

#include <optional>

#include "ast/base/node_ptr.h"
#include "forInitStmt.h"
#include "ast/expr/expr.h"
#include "Condition.h"
#include "stmt.h"

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents the for-loop statement.
     *
     * This does not represent the for-range loop.
     */
    class ForStmt : public Node {
    public:
        ForStmt(SourceInfo src_info, node_ptr<ForInitStmt> initStmt,
                std::optional<node_ptr<Condition>> cond,
                std::optional<node_ptr<Expr>> post_iter, node_ptr<Stmt> body) :
                Node(std::move(src_info)),
                initStmt(std::move(initStmt)),
                cond(std::move(cond)),
                post_iter(std::move(post_iter)),
                body(std::move(body)) {}

        node_ptr<ForInitStmt> initStmt;
        std::optional<node_ptr<Condition>> cond;
        std::optional<node_ptr<Expr>> post_iter;
        node_ptr<Stmt> body;
    };
}

