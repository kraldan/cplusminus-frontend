#pragma once

#include <string>
#include <optional>

#include "ast/base/Node.h"
#include "ast/decl/Decl.h"

namespace ast {
    /**
     * Represents an identifier used in an expression context.
     *
     * Example:
     * 'a + 5', the 'a' will be an IdExpr
     *
     * Note: after parsing, before semantic analysis, this can refer to variables OR
     * non-static class members that are implicitly accessed inside a method.
     * After semantic analysis, this only refers to values available in current scope,
     * not to non-static class members.
     */
    class IdExpr : public Node {
    public:
        explicit IdExpr(SourceInfo src_info, std::string id);

        std::string id;
        // this determines to which value this id expression corresponds,
        // this is set during semantic analysis
        std::optional<const ast::Decl *> var = std::nullopt;
    };
}