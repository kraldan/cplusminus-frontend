#pragma once

#include <variant>

#include "SimpleDeclaration.h"
#include "ast/func/FuncDef.h"
#include "ast/class/ClassDef.h"
#include "EmptyDeclaration.h"

namespace ast {
    /**
     * Represents all declarations that can happen in the global scope.
     */
    using Declaration = std::variant<
            SimpleDeclar,
            FuncDef,
            ClassDef,
            EmptyDeclaration
    >;
}