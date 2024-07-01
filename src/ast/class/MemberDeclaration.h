#pragma once

#include <variant>
#include "ast/func/FuncDef.h"
#include "ast/class/MemberDeclaratorList.h"
#include "ast/decl/EmptyDeclaration.h"

namespace ast {
    using MemberDeclaration = std::variant<
            MemberDeclaratorList,
            FuncDef
    >;
}