#pragma once

#include <cstdint>


#include "ast/base/Node.h"

namespace ast {

    /**
     * Represents an integer literal.
     *
     * Example:
     *  '42'
     */
    class IntLiteral : public Node {
    public:
        explicit IntLiteral(SourceInfo src_info, uint64_t val);

        uint64_t val;
    };
}
