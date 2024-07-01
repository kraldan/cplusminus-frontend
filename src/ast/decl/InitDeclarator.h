#pragma once

#include <optional>

#include "ast/base/node_ptr.h"
#include "ast/expr/expr.h"
#include "Decl.h"

#include "ast/base/Node.h"

namespace ast {

    /**
     * Represents declaration (and possible initialization) of a variable or
     * a function forward declaration.
     */
    class InitDeclarator : public Node {
    public:
        explicit InitDeclarator(SourceInfo src_info, node_ptr<Decl> declarator,
        std::optional<node_ptr<Expr>> initializer) :
		Node(std::move(src_info)),
        declarator(std::move(declarator)),
        initializer(std::move(initializer))
        {}

        node_ptr<Decl> declarator;
        // expression for now
        std::optional<node_ptr<Expr>> initializer;
    };
}