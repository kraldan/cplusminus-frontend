#pragma once

#include <vector>

#include "ast/base/Node.h"
#include "ast/base/node_ptr.h"
#include "MemberDeclarator.h"

namespace ast {
    /**
     * Represents list of class members declarators which share declaration specifier sequence.
     *
     * Example:
     * In
     * '
     * struct S {
     *      int a, *p, *pp;
     * };
     * '
     * the 'int a, *p, *pp' is a MemberDeclaratorList
     */
    class MemberDeclaratorList : public Node {
    public:
        MemberDeclaratorList(SourceInfo src_info, std::vector<node_ptr<MemberDeclarator>> decls) :
                Node(std::move(src_info)),
                decls(std::move(decls)) {}

        // clang issues with vector
        MemberDeclaratorList(MemberDeclaratorList &&) = default;

        std::vector<node_ptr<MemberDeclarator>> decls;
    };
}
