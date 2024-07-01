#include "TypeManager.h"

using namespace std;

template<typename D, typename... Args>
D *cpm::TypeManager::getDerivedType(D *(*cast_func)(cpm::Type *), Args... args) {
    D tmp = D(args...);
    std::string repr = cpm::repr_type(&tmp);
    if (types.contains(repr))
        return cast_func(types.at(repr).get());
    unique_ptr<D> up = make_unique<D>(tmp);
    D *ptr = up.get();
    types[repr] = std::move(up);
    return ptr;
}

cpm::SimpleType *cpm::TypeManager::getSimpleType(const std::string &type_id, bool is_const) {
    return getDerivedType<cpm::SimpleType>(cpm::simple_ty, type_id, is_const);
}

cpm::PointerType *cpm::TypeManager::getPointerType(cpm::Type *elem_type, bool is_const) {
    return getDerivedType<cpm::PointerType>(cpm::pointer_ty, elem_type, is_const);
}

cpm::ArrayType *cpm::TypeManager::getArrayType(cpm::Type *elem_type, std::optional<size_t> size) {
    return getDerivedType<cpm::ArrayType>(cpm::array_ty, elem_type, size);
}

cpm::FunctionType *
cpm::TypeManager::getFunctionType(cpm::Type *ret_type, std::vector<cpm::Type *> params,
                                  bool is_vararg) {
    return getDerivedType<cpm::FunctionType>(cpm::function_ty, ret_type, std::move(params),
                                             is_vararg);
}
