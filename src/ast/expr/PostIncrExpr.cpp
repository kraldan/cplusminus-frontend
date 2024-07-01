#include "PostIncrExpr.h"

ast::PostIncrExpr::PostIncrExpr(ast::SourceInfo src_info, ast::node_ptr<ast::Expr> expr, bool incr)
        :
        Node(std::move(src_info)),
        expr(std::move(expr)),
        incr(incr) {}
