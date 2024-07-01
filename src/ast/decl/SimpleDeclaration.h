#pragma once

#include <vector>

#include "ast/base/node_ptr.h"
#include "InitDeclarator.h"

#include "ast/base/Node.h"

namespace ast {
    /**
     * Represents multiple variable/function declarations that share
     * declaration specifier sequence.
     *
     * Example:
     * 'int a = 5, foo(char c), *p, **pp = nullptr'
     * all of this is covered by a single SimpleDeclar.
     * The shared declaration specifier sequence is 'int'.
     */
    class SimpleDeclar : public Node {
    public:
        SimpleDeclar(SourceInfo src_info,
                     std::vector<node_ptr<InitDeclarator>>
                     init_declars) :
                Node(std::move(src_info)),
                init_declars(std::move(init_declars)) {}

        // clang issues with vector
        SimpleDeclar(SimpleDeclar &&) = default;

        std::vector<node_ptr<InitDeclarator>> init_declars;
    };

}