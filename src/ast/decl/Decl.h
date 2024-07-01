#pragma once

#include <string>

#include "ast/base/Node.h"
#include "type/Type.h"

namespace ast {

    /**
     * Represents a declarator.
     *
     * This can be a variable declarator, parameter declarator,
     * member declarator...
     *
     * Example:
     * in
     * 'int f(char c, double d, void *p)'
     * there are 4 declarators (id, type):
     *      f   int (char, double, void*)
     *      c   char
     *      d   double
     *      p   void*
     */
    class Decl : public Node {
    public:
        Decl(SourceInfo src_info, cpm::Type *type, std::string id);

        // ast::FunctionDecl inherits from this
        virtual ~Decl() = default;

        cpm::Type *type;
        std::string id;
    };

}