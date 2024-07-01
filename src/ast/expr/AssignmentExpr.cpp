#include "AssignmentExpr.h"

ast::AssignmentExpr::AssignmentExpr(ast::SourceInfo src_info, ast::node_ptr<ast::Expr> dest,
                                    ast::node_ptr<ast::Expr> val, ast::AssignOp op) :
        Node(std::move(src_info)),
        lhs(std::move(dest)),
        rhs(std::move(val)),
        op(op) {}