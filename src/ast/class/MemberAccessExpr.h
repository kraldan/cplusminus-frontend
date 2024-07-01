#pragma once

#include <string>

#include "ast/base/Node.h"
#include "ast/base/node_ptr.h"
#include "ast/expr/expr.h"
#include "ast/expr/IdExpr.h"

namespace ast {
    /**
     * Represents an accession to a class field.
     *
     * Example:
     * 's.a', 's->b'
     * */
    class MemberAccessExpr : public Node {
    public:
        MemberAccessExpr(SourceInfo src_info, node_ptr <Expr> object, bool ptr_access,
                         std::string member) :
                Node(std::move(src_info)),
                object(std::move(object)),
                ptr_access(ptr_access),
                member(std::move(member)) {}

        node_ptr<Expr> object;
        // whether it was '->' or '.'
        bool ptr_access;
        std::string member;
    };
}
