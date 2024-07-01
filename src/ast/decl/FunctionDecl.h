#pragma once

#include <vector>

#include "ast/decl/Decl.h"
#include "ast/func/Param.h"
#include "type/DerivedTypes.h"

namespace ast {

    // resolve cyclical inclusion
    class Param;

    /**
     * Represents a function declarator.
     *
     * Example:
     * int foo(int a, char c = 'a') // more code
     *
     */
    class FunctionDecl : public Decl {
    public:
        FunctionDecl(SourceInfo src_info, cpm::FunctionType *func_type, std::string id,
                     std::vector<node_ptr<Param>> params);

        // clang issues with vector
        FunctionDecl(FunctionDecl &&) = default;

        std::vector<node_ptr<Param>> params;

        // pointer to the first declaration of this function, if this is not the first
        // set during semantic analysis
        std::optional<const ast::FunctionDecl *> orig = std::nullopt;
    };
}