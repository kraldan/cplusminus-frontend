#include "IdExpr.h"

ast::IdExpr::IdExpr(ast::SourceInfo src_info, std::string id) :
        Node(std::move(src_info)),
        id(std::move(id)) {}
