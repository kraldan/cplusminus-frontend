#include "BinaryExpr.h"

ast::BinaryExpr::BinaryExpr(ast::SourceInfo src_info, ast::node_ptr<ast::Expr> left,
                            ast::node_ptr<ast::Expr> right,
                            ast::BinaryOp op) :
        Node(std::move(src_info)),
        lhs(std::move(left)),
        rhs(std::move(right)),
        op(op) {}
