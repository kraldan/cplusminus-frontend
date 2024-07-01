#pragma once

#include <utility>
#include <memory>

namespace ast {

    /**
     * This is used in ast nodes to contain child nodes.
     */
    template<class T>
    using node_ptr = std::unique_ptr<T>;

    /**
     * Create a node pointer of given type, optionally as a variant.
     * @tparam T type of ast node
     * @tparam Var variant that contains T, or T
     * @tparam Args
     * @param args arguments for constructor of T
     * @return
     */
    template<class T, typename Var = T, typename ... Args>
    node_ptr<Var> make_node(Args ... args) {
        Var v = T(std::forward<Args>(args)...);
        return std::make_unique<Var>(std::move(v));
    }

    /**
     * Change node_ptr containing T to node_ptr containing V.
     * @tparam T
     * @tparam Var
     * @param ptr
     * @return
     */
    template<typename T, typename Var>
    node_ptr<Var> change_node(node_ptr<T> ptr) {
        return make_node<T, Var>(std::move(*ptr));
    }
}
