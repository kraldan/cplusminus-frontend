#pragma once

#include <optional>

#include "ast/base/node_ptr.h"
#include "expr.h"

#include "ast/base/Node.h"
#include "ast/decl/FunctionDecl.h"

namespace ast {

    // resolve cyclical inclusion in ast files
    class FunctionDecl;

    /**
     * Represents a function call.
     *
     * This can be a function call, method call or constructor call.
     *
     * Examples:
     * 'f(a, b, 5)'
     * 'obj->foo()'
     * 'S(42)'
     */
    class CallExpr : public Node {
    public:
        CallExpr(SourceInfo src_info, node_ptr<Expr> callee, std::vector<node_ptr<Expr>> args);

        // clang issues with vector
        CallExpr(CallExpr &&) = default;


        // this is ast::IdExpr or ast::MemberAccessExpr
        node_ptr<Expr> called_func;
        std::vector<node_ptr<Expr>> args;
        // whether this is a constructor call or not; this should be set during semantic analysis
        bool ctor_call = false;
        // which function does this call refer to? set during semantic analysis after overload
        // resolution
        std::optional<const ast::FunctionDecl *> func = std::nullopt;
    };
}

