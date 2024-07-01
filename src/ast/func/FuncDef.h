#pragma once

#include <optional>

#include "ast/decl/Decl.h"
#include "ast/decl/FunctionDecl.h"
#include "FuncBody.h"

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents a function definition.
     *
     * This can be a global function, non-static class method or a constructor.
     *
     * Example:
     * 'int foo() {
     *      return 42;
     *  }
     * '
     *
     */
    class FuncDef : public Node {
    public:
        FuncDef(SourceInfo src_info, node_ptr<FunctionDecl> declarator, node_ptr<FuncBody> body,
                bool ctor) :
                Node(std::move(src_info)),
                declarator(std::move(declarator)),
                body(std::move(body)),
                ctor(ctor) {}

        node_ptr<FunctionDecl> declarator;
        node_ptr<FuncBody> body;
        // is this a constructor?
        bool ctor;
    };
}

