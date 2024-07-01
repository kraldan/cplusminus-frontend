#pragma once

#include <string>

#include "ast/base/Node.h"

namespace ast {
    enum ClassKey {
        Class,
        Struct
    };

    /**
     * Represents the head of a class - class name and whether it's a class or struct.
     */
    class ClassHead : public Node {
    public:
        ClassHead(SourceInfo src_info, ClassKey key, std::string name) :
                Node(std::move(src_info)),
                key(key),
                name(std::move(name)) {}

        ClassKey key;
        std::string name;
    };
}
