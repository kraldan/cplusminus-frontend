#include "UnaryExpr.h"

ast::UnaryExpr::UnaryExpr(ast::SourceInfo src_info, ast::UnaryOp op, ast::node_ptr<ast::Expr> expr)
        :
        Node(std::move(src_info)),
        op(op),
        expr(std::move(expr)) {}