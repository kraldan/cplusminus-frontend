#pragma once

#include <vector>

#include "ast/base/node_ptr.h"
#include "ast/decl/declaration.h"

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents a translation unit.
     */
    class TranslationUnit : public Node {
    public:
        explicit TranslationUnit(SourceInfo src_info,
                                 std::vector<node_ptr < Declaration>>

        declars) :

        Node (std::move(src_info)),
        declars(std::move(declars)) {}

        // clang issues with vector
        TranslationUnit(TranslationUnit &&) = default;

        std::vector<node_ptr<Declaration>> declars;
    };
}

