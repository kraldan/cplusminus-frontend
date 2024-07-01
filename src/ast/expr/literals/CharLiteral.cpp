#include "CharLiteral.h"

ast::CharLiteral::CharLiteral(ast::SourceInfo src_info, char c) :
        Node(std::move(src_info)),
        c(c) {}
