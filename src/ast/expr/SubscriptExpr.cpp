#include "SubscriptExpr.h"

ast::SubscriptExpr::SubscriptExpr(ast::SourceInfo src_info, ast::node_ptr<ast::Expr> dest,
                                  ast::node_ptr<ast::Expr> index) :
        Node(std::move(src_info)),
        dest(std::move(dest)),
        index(std::move(index)) {}
