#pragma once

#include <map>
#include <memory>
#include <string>

#include "type/Type.h"
#include "type/DerivedTypes.h"

namespace cpm {
    /**
     * A type 'context' that manages types.
     */
    class TypeManager {
    public:
        TypeManager() = default;

        cpm::SimpleType *getSimpleType(const std::string &type_id, bool is_const);

        cpm::PointerType *getPointerType(cpm::Type *elem_type, bool is_const);

        cpm::ArrayType *getArrayType(cpm::Type *elem_type, std::optional<size_t> size);

        cpm::FunctionType *getFunctionType(cpm::Type *ret_type, std::vector<cpm::Type *> params,
                                           bool is_vararg);

    private:
        std::map<std::string, std::unique_ptr<cpm::Type>> types;

        // helper for the funcs above
        template<typename T, typename ... Args>
        T *getDerivedType(T *(*cast_func)(cpm::Type *), Args... args);
    };
}
