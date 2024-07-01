#include "Decl.h"


ast::Decl::Decl(SourceInfo src_info, cpm::Type *type, std::string id) :
        Node(std::move(src_info)),
        type(type),
        id(id) {}
