#pragma once

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents an empty declaration, extraneous semicolon.
     */
    class EmptyDeclaration : public Node {
    public:
        explicit EmptyDeclaration(SourceInfo src_info) :
                Node(std::move(src_info)) {};
    };
}
