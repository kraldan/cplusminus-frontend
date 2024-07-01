#pragma once

#include <optional>

#include "ast/base/node_ptr.h"
#include "ast/expr/expr.h"

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents a return statement.
     */
    class ReturnStmt : public Node {
    public:
        explicit ReturnStmt(SourceInfo src_info, std::optional<node_ptr<Expr>> expr) :
		Node(std::move(src_info)),
        expr(std::move(expr)) {}

        std::optional<node_ptr<Expr>> expr;
    };
}
