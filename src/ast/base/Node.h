#pragma once

#include <utility>

#include "SourceInfo.h"

namespace ast {


    /**
     * This struct holds SourceInfo information and is contained in all ast classes.
     *
     * It is not meant to be used for polymorphism; it is intentionally NOT polymorphic.
     * */
    struct Node {
        explicit Node(SourceInfo src_info);

        // DO NOT MAKE THIS TYPE POLYMORPHIC

        SourceInfo src_info;
    };
}

