#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cassert>
#include <stdexcept>

#include "Type.h"

namespace cpm {

    class SimpleType : public Type {
        std::string type_id;
        bool is_const;

    public:
        SimpleType(std::string type_id, bool is_const) :
                type_id(std::move(type_id)),
                is_const(is_const) {}

        const std::string &getTypeId() const { return type_id; }

        bool isConst() const { return is_const; }

        bool operator<(const SimpleType &rhs) const {
            return type_id < rhs.type_id;
        }
    };

    class PointerType : public Type {
        Type *elem_type;
        bool is_const;

    public:

        PointerType(Type *elem_type, bool is_const) :
                elem_type(elem_type),
                is_const(is_const) {}

        Type *getElemType() const { return elem_type; }

        bool isConst() const { return is_const; }
    };

    class FunctionType : public Type {
        Type *ret_type;
        std::vector<Type *> params;
        bool vararg;

    public:
        FunctionType(Type *ret_type,
                     std::vector<Type *> params,
                     bool vararg) :
                ret_type(ret_type),
                params(std::move(params)),
                vararg(vararg) {}

        Type *getRetType() const { return ret_type; }

        const std::vector<Type *> &getParams() const { return params; };

        bool isVararg() const { return vararg; }
    };

    class ArrayType : public Type {
        Type *elem_type;
        std::optional<size_t> size;

    public:
        ArrayType(Type *elem_type, std::optional<size_t> size) :
                elem_type(elem_type),
                size(size) {}

        Type *getElemType() const { return elem_type; }

        const std::optional<size_t> &getSize() const { return size; }
    };
    /*
     * Helper func to cast Type to a derived types.
     *
     * As the inner implementation of Type might change, use exclusively
     * those (instead of dynamic cast or something like that).
     */

    /* Return pointer to SimpleType object if t is SimpleType, nullptr otherwise. */
    SimpleType *simple_ty(Type *t);

    /* Return pointer to PointerType object if t is PointerType, nullptr otherwise. */
    PointerType *pointer_ty(Type *t);

    /* Return pointer to ArrayType object if t is ArrayType, nullptr otherwise. */
    ArrayType *array_ty(Type *t);

    /* Return pointer to FunctionType object if t is FunctionType, nullptr otherwise. */
    FunctionType *function_ty(Type *t);

    /* Returns a unique string represenanntation for each possible type.
     * Suggested use: key in map */
    std::string repr_type(Type *t);

    /* Returns a human-readable string for given type. */
    std::string to_string(Type *t);

    std::string to_string(FunctionType *t);

}
