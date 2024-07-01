#pragma once

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represent the bool literals 'true' and 'false'.
     */
    class BoolLiteral : public Node {
    public:
        explicit BoolLiteral(SourceInfo src_info, bool val);

        bool val;
    };
}
