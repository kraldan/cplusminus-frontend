#pragma once

#include <variant>

#include "ExprStmt.h"
#include "ast/decl/SimpleDeclaration.h"

namespace ast {
    /**
     * Represents options of the for-loop initializer statement.
     */
    using ForInitStmt = std::variant<
            ExprStmt,
            SimpleDeclar
    >;
}

