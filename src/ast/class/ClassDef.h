#pragma once

#include "ast/base/Node.h"
#include "ast/base/node_ptr.h"
#include "ClassHead.h"
#include "MemberSpecification.h"

namespace ast {
    /**
     * Represents the entire definition of a class or struct.
     *
     * Example:
     * class S {
     *      int i;
     *
     *  public:
     *      int some_method() {return i;}
     * };
     */
    class ClassDef : public Node {
    public:
        ClassDef(SourceInfo src_info, node_ptr<ClassHead> head,
                 std::optional<node_ptr<MemberSpecification>> body) :
                Node(std::move(src_info)),
                head(std::move(head)),
                body(std::move(body)) {}

        node_ptr<ClassHead> head;
        std::optional<node_ptr<MemberSpecification>> body;
    };
}
