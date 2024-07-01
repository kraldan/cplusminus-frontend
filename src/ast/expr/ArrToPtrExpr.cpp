//
// Created by micro on 2/22/2024.
//

#include "ArrToPtrExpr.h"

ast::ArrToPtrExpr::ArrToPtrExpr(ast::SourceInfo src_info, ast::node_ptr<ast::Expr> arr_expr) :
        Node(std::move(src_info)),
        arr_expr(std::move(arr_expr)) {}
