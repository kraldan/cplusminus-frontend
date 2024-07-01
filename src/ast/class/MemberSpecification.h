#pragma once

#include <vector>
#include <variant>

#include "ast/base/Node.h"
#include "ast/base/node_ptr.h"
#include "MemberDeclaration.h"
#include "AccessModifier.h"

namespace ast {

    /* Everything inside the class body is either a member declaration or an access modifier. */
    using MemberSpecElem = std::variant<MemberDeclaration, AccessModifier>;

    /**
     * Represents all the declarations, definitions and access modifiers in a class.
     *
     * The entire class body is under this.
     */
    class MemberSpecification : public Node {
    public:
        MemberSpecification(SourceInfo src_info, std::vector<node_ptr<MemberSpecElem>> list) :
                Node(std::move(src_info)),
                list(std::move(list)) {}

        // clang issues with vector
        MemberSpecification(MemberSpecification &&) = default;

        std::vector<node_ptr<MemberSpecElem>> list;
    };
}
