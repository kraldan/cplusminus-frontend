#pragma once

#include <map>
#include <vector>
#include <string>

#include "type/Type.h"
#include "type/DerivedTypes.h"
#include "ast/expr/DefaultArgExpr.h"
#include "ScopeValue.h"


namespace cpm::sc {
/**
 * This represents a scope for the type checker.
 */
    class Scope {
    protected:
        Scope *parent;
        std::map<std::string, std::vector<ScopeValue>> values;
        /* named subscopes */
        std::map<std::string, Scope *> named_children;

    public:
        explicit Scope(Scope *parent) :
                parent(parent) {}

        virtual ~Scope() = default;

        std::vector<ScopeValue> getValues(const std::string &id, bool search_in_parent) {
            if (contains(id))
                return values[id];
            else if (parent && search_in_parent)
                return parent->getValues(id, search_in_parent);
            return {};
        }

        /**
         *
         * @param id
         * @param type
         */
        void addValue(const std::string &id, cpm::Type *type, const ast::Decl *decl) {
            values[id].push_back({decl, type});
        }

        /**
         * If value of given id exists in this scope or its ancestors, return
         * pointer to the scope where it exists.
         * @param id
         * @return
         */
        Scope *getValueScope(const std::string &id);

        /**
         * Checks whether this scope contains a value with 'id'.
         *
         * Does not check in parent scopes.
         * @param id
         * @return
         */
        bool contains(const std::string &id) {
            return values.contains(id);
        }

        [[nodiscard]] Scope *getParent() const {
            return parent;
        }

        void setParent(Scope *new_parent) {
            parent = new_parent;
        }
    };
}
