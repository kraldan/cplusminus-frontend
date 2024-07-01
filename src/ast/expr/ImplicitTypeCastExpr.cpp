#include "ImplicitTypeCastExpr.h"

ast::ImplicitTypeCastExpr::ImplicitTypeCastExpr(ast::SourceInfo src_info,
                                                ast::node_ptr<ast::Expr> val,
                                                cpm::Type *dest_ty) :
        Node(std::move(src_info)),
        val(std::move(val)),
        dest_ty(dest_ty) {}