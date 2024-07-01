#include "TernaryExpr.h"

ast::TernaryExpr::TernaryExpr(ast::SourceInfo src_info, ast::node_ptr<ast::Expr> cond,
                              ast::node_ptr<ast::Expr> then,
                              ast::node_ptr<ast::Expr> else_) :
        Node(std::move(src_info)),
        cond(std::move(cond)),
        then(std::move(then)),
        else_(std::move(else_)) {}
