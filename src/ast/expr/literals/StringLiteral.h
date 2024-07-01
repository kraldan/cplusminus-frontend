#pragma once

#include <string>

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents a string literal.
     *
     * Example:
     * "Ahoj, svete!"
     */
    class StringLiteral : public Node {
    public:
        explicit StringLiteral(SourceInfo src_info, std::string str);

        std::string str;
    };
}
