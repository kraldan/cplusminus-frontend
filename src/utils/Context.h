#pragma once

#include <vector>
#include <string>
#include <istream>

#include "type/DerivedTypes.h"
#include "type/TypeManager.h"

namespace cpm {
    /**
     * Represents context for compilation of a C+- source file.
     */
    class Context {
    public:
        explicit Context(std::istream &ifs);

        const std::string &getInput() const;

        const std::vector<std::string> &getInputByLines() const;

        cpm::SimpleType *getSimpleType(const std::string &type_id, bool is_const);

        cpm::PointerType *getPointerType(cpm::Type *elem_type, bool is_const);

        cpm::ArrayType *getArrayType(cpm::Type *elem_type, std::optional<size_t> size);

        cpm::FunctionType *getFunctionType(cpm::Type *ret_type, std::vector<cpm::Type *> params,
                                           bool is_vararg);

    private:
        TypeManager tm;
        std::string input;
        std::vector<std::string> lines;
    };

}
