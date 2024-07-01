#include "LValToRValExpr.h"

ast::LValToRValExpr::LValToRValExpr(ast::SourceInfo src_info, ast::node_ptr<ast::Expr> val) :
        Node(std::move(src_info)),
        val(std::move(val)) {}
