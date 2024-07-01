#pragma once

#include "ast/decl/SimpleDeclaration.h"
#include "ast/base/node_ptr.h"

#include "ast/base/Node.h"

namespace ast {

    /**
     * Represents a declaration statement.
     *
     * For example:
     * void f {
     *      int a;      // this is a declaration statement
     * }
     */
    class DeclarStmt : public Node {
    public:
        explicit DeclarStmt(SourceInfo src_info, node_ptr<SimpleDeclar> declar) :
		Node(std::move(src_info)),
        declaration(std::move(declar))
        {}

        node_ptr<SimpleDeclar> declaration;
    };
}
