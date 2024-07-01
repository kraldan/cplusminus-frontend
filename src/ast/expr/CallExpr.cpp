#include "CallExpr.h"

ast::CallExpr::CallExpr(SourceInfo src_info, node_ptr<Expr> callee,
                        std::vector<node_ptr<Expr>> args)
        :
        Node(std::move(src_info)),
        called_func(std::move(callee)),
        args(std::move(args)) {}