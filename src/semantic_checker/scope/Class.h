#pragma once

#include <utility>
#include <set>
#include "ast/class/AccessModifier.h"

#include "Scope.h"

namespace cpm::sc {
    using id_type = std::pair<std::string, cpm::Type *>;

/**
 * This represents a class scope.
 */
    class Class : public Scope {
        std::set<const ast::Decl *> public_members;
        std::set<const ast::Decl *> private_members;

    public:

        explicit Class(Scope *parent) :
                Scope(parent) {}

        void addValue(const std::string &id, cpm::Type *type, const ast::Decl *decl,
                      ast::AccessModifier am) {
            if (am == ast::PUBLIC)
                public_members.emplace(decl);
            else if (am == ast::PRIVATE)
                private_members.emplace(decl);
            return Scope::addValue(id, type, decl);
        }

        /**
         * Returns true if class member with given id and type is public.
         * Returns false if it is not public or doesn't exist.
         */
        bool is_public(const ast::Decl *decl) {
            return public_members.contains(decl);
        }

        /**
         * Returns true if class member with given id and type is private.
         * Returns false if it is not private or doesn't exist.
         */
        bool is_private(const ast::Decl *decl) {
            return private_members.contains(decl);
        }
    };
}


