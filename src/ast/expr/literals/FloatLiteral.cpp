#include "FloatLiteral.h"

ast::FloatLiteral::FloatLiteral(ast::SourceInfo src_info, double val) :
        Node(std::move(src_info)),
        val(val) {}