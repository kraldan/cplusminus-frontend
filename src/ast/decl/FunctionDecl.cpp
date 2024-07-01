#include "FunctionDecl.h"


ast::FunctionDecl::FunctionDecl(SourceInfo src_info, cpm::FunctionType *func_type, std::string id,
                                std::vector<node_ptr<Param>> params) :
        ast::Decl(std::move(src_info), func_type, std::move(id)),
        params(std::move(params)) {}
