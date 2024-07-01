#include "ast/decl/Decl.h"
#include "type/Type.h"

namespace cpm::sc {
    /**
     * Represents a value stored in scope.
     */
    struct ScopeValue {
        // pointer to the node by which this value was declared
        const ast::Decl *decl;
        // type of the value
        Type *type;

        // note on value category: since this is saved in a scope under a name, this is
        // always implicitly lvalue
    };
}
