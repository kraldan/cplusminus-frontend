#include "DerivedTypes.h"
#include <cassert>

namespace cpm {

    template<typename Derived>
    Derived *cast(Type *t) {
        return dynamic_cast<Derived *>(t);
    }

    SimpleType *simple_ty(Type *t) {
        return cast<SimpleType>(t);
    }

    PointerType *pointer_ty(Type *t) {
        return cast<PointerType>(t);
    }

    ArrayType *array_ty(Type *t) {
        return cast<ArrayType>(t);
    }

    FunctionType *function_ty(Type *t) {
        return cast<FunctionType>(t);
    }

    std::string repr_type(Type *t) {
        using namespace std;
        // separates semantically different parts of type
        auto sep_str = "@";
        string sep;
        sep = sep_str;
        if (auto s = simple_ty(t)) {
            string c = s->isConst() ? "C" + sep : "";
            return c + s->getTypeId();
        } else if (auto p = pointer_ty(t)) {
            string c = p->isConst() ? "C" + sep : "";
            return c + "P" + sep + repr_type(p->getElemType());
        } else if (auto a = array_ty(t)) {
            long long size = a->getSize().has_value() ? (long long) a->getSize().value() : -1;
            return "A" + sep + std::to_string(size) + sep + repr_type(a->getElemType());
        } else if (auto f = function_ty(t)) {
            string repr = "F" + sep + repr_type(f->getRetType());
            for (const auto &pt: f->getParams())
                repr += sep + repr_type(pt);
            if (f->isVararg())
                repr += sep + "...";
            return repr;
        } else if (t == nullptr)
            assert(false && "got nullptr");
        assert(false && "unknown type in repr_type");
    }

    std::string _to_string(Type *t);

    std::string _to_string(FunctionType *f);

    std::string _to_string(Type *t) {
        if (auto r = simple_ty(t))
            return (r->isConst() ? "const " : "") + r->getTypeId();
        else if (auto p = pointer_ty(t))
            return (p->isConst() ? "const ptr to " : "ptr to ") + _to_string(p->getElemType());
        else if (auto a = array_ty(t)) {
            std::string size_str = a->getSize().has_value() ?
                                   std::to_string(a->getSize().value()) :
                                   "unknown size";
            return "[" + size_str + " x " + _to_string(a->getElemType()) + "]";
        } else if (auto f = function_ty(t)) {
            return _to_string(f);
        } else if (t == nullptr)
            throw std::runtime_error("idk::_to_string unexpected nullptr");
        else
            throw std::runtime_error("idk::_to_string unexpected type");
    }


    /*
    std::string _to_string2(Type * t);
    std::string _to_string2(SimpleType * t) {
        return (t->isConst() ? "const " : "") + t->getTypeId();
    }
    std::string _to_string2(PointerType * t) {
        return "_to_string2 unhandled for PointerType";
    }
    std::string _to_string2(ArrayType * t) {
        int s = t->getSize().has_value() ? t->getSize().value() : -1;
        Type *elem_ty = t->getElemType();
        std::string elem_str = _to_string2(elem_ty);
        std::string brackets = "[" + std::to_string(s) + "]";
        if(simple_ty(elem_ty) || pointer_ty(elem_ty))
            return elem_str + brackets;
        else if(array_ty(elem_ty)) {
            size_t i = elem_str.find("[");
            assert(i != std::string::npos);
            elem_str.insert(i, brackets);
            return elem_str;
        }
        return "array of functions is unhandled";
    }
    std::string _to_string2(FunctionType * t) {
        std::string res = _to_string2(t->getRetType());
        res += " (";
        for(auto * t : t->getParams())
            res += _to_string2(t) + ", ";
        res += ")";
    }
    */

    std::string _to_string(FunctionType *f) {
        std::string s = _to_string(f->getRetType()) + " (";
        for (size_t i = 0; i < f->getParams().size(); i++) {
            if (i)
                s += ", ";
            s += _to_string(f->getParams()[i]);
        }
        if (f->isVararg())
            s += ", ...";
        s += ")";
        return s;
    }

    std::string to_string(Type *t) {
        return "'" + _to_string(t) + "'";
    }

    std::string to_string(FunctionType *ft) {
        return "'" + _to_string(ft) + "'";
    }
}

