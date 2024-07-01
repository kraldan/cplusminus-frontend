#include "BoolLiteral.h"

ast::BoolLiteral::BoolLiteral(ast::SourceInfo src_info, bool val) :
        Node(std::move(src_info)),
        val(val) {}
