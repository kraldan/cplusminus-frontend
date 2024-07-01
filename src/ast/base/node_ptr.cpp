#include "node_ptr.h"

// this file is currently unused (12.3.24)

namespace ast {
//    template <class T, typename Var, typename ... Args>
//    node_ptr<Var> make_node(Args ... args) {
//        Var v = T(std::forward<Args>(args)...);
//        return std::make_unique<Var>(std::move(v));
//    }

    template<typename T, typename Var>
    node_ptr<Var> change_node(node_ptr<T> ptr) {
        return make_node<T, Var>(std::move(*ptr));
    }

}
