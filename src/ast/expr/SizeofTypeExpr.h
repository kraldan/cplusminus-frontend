#pragma once

#include "type/Type.h"

#include "ast/base/Node.h"
#include "ast/base/node_ptr.h"

namespace ast {
    /* Represents the expression 'sizeof(type)' */
    class SizeofTypeExpr : public Node {
    public:
        SizeofTypeExpr(SourceInfo src_info, cpm::Type *type) :
                Node(std::move(src_info)),
                type(type) {}

        cpm::Type *type;
    };
}
