#pragma once

#include "ast/base/Node.h"

namespace ast {
    /** Represents the nullptr literal. */
    class NullptrLiteral : public Node {
    public:
        explicit NullptrLiteral(SourceInfo src_info);
    };
}

