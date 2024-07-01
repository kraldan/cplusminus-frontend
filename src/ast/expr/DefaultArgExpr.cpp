#include "DefaultArgExpr.h"

ast::DefaultArgExpr::DefaultArgExpr(ast::SourceInfo src_info, ast::Expr *expr) :
        Node(std::move(src_info)),
        expr(expr) {}