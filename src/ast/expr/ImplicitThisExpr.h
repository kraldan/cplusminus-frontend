#pragma once

#include "ast/base/Node.h"

namespace ast {

    /**
     * Represents the implicit 'this->' when class members are accessed within class methods.
     *
     * Example:
     * '
     * struct S {
     *      int a;
     *      int foo() {
     *          return a;
     *      }
     * };
     * '
     * the 'a' in foo has an implicit 'this->'.
     */
    class ImplicitThisExpr : public Node {
    public:
        explicit ImplicitThisExpr(SourceInfo src_info);
    };

}