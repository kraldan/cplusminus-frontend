#include "StringLiteral.h"

ast::StringLiteral::StringLiteral(ast::SourceInfo src_info, std::string str) :
        Node(std::move(src_info)),
        str(std::move(str)) {}
