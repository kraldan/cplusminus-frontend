#include <sstream>
#include "Context.h"

namespace cpm {
    Context::Context(std::istream &ifs) :
            input(std::string(std::istreambuf_iterator<char>(ifs),
                              std::istreambuf_iterator<char>())),
            lines{"line 0, this should not be accessed"} {
        std::stringstream ss(input);
        std::string line;
        while (std::getline(ss, line, '\n'))
            lines.push_back(line);
    }

    cpm::SimpleType *Context::getSimpleType(const std::string &type_id, bool is_const) {
        return tm.getSimpleType(type_id, is_const);
    }

    cpm::PointerType *Context::getPointerType(cpm::Type *elem_type, bool is_const) {
        return tm.getPointerType(elem_type, is_const);
    }

    cpm::ArrayType *Context::getArrayType(cpm::Type *elem_type, std::optional<size_t> size) {
        return tm.getArrayType(elem_type, size);
    }

    cpm::FunctionType *
    Context::getFunctionType(cpm::Type *ret_type, std::vector<cpm::Type *> params, bool is_vararg) {
        return tm.getFunctionType(ret_type, std::move(params), is_vararg);
    }

    const std::string &Context::getInput() const {
        return input;
    }

    const std::vector<std::string> &Context::getInputByLines() const {
        return lines;
    }
}