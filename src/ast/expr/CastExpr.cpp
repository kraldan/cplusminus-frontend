#include "CastExpr.h"

ast::CastExpr::CastExpr(ast::SourceInfo src_info, cpm::Type *type, ast::node_ptr<ast::Expr> expr) :
        Node(std::move(src_info)),
        type(type),
        expr(std::move(expr)) {}
