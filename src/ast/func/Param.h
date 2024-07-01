#pragma once

#include <vector>
#include <optional>

#include "ast/base/node_ptr.h"
#include "ast/expr/expr.h"
#include "ast/decl/Decl.h"

#include "ast/base/Node.h"

namespace ast {

    // resolve cyclical inclusion in ast files
    class Decl;

    /**
     * Represents a function parameter declaration.
     *
     * Example:
     * 'int foo(int a = 5)'
     * Param represents the 'int a = 5'
     */
    class Param : public Node {
    public:
        Param(SourceInfo src_info,
                  node_ptr<Decl> declarator,
                  std::optional<node_ptr<Expr>> default_val) :
		        Node(std::move(src_info)),
                declarator(std::move(declarator)),
                default_val(std::move(default_val))
        {}

        node_ptr<Decl> declarator;
        std::optional<node_ptr<Expr>> default_val;
    };
}
