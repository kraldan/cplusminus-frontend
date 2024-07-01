#pragma once


#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents a character literal.
     *
     * Example:
     * "'c'"
     */
    class CharLiteral : public Node {
    public:
        explicit CharLiteral(SourceInfo src_info, char c);

        char c;
    };
}
