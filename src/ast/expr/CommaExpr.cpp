#include "CommaExpr.h"

ast::CommaExpr::CommaExpr(SourceInfo src_info, std::vector<node_ptr<Expr>> exprs)
        :
        Node(std::move(src_info)),
        expressions(std::move(exprs)) {}