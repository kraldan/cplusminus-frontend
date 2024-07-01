#include "IntLiteral.h"

ast::IntLiteral::IntLiteral(ast::SourceInfo src_info, uint64_t val) :
        Node(std::move(src_info)),
        val(val) {}
