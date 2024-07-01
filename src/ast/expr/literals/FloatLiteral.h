#pragma once

#include "ast/base/Node.h"

namespace ast {

    /**
     * Represents a floating point number literal.
     *
     * Example:
     * '3.14'
     */
    class FloatLiteral : public Node {
    public:
        FloatLiteral(SourceInfo src_info, double val);

        double val;
    };
}

