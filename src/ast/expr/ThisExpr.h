#pragma once

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents the explicit 'this' expression that can be
     * used inside class methods.
     */
    class ThisExpr : public Node {
    public:
        explicit ThisExpr(SourceInfo src_info);
    };
}

