#include "Node.h"

ast::Node::Node(ast::SourceInfo src_info) :
        src_info(std::move(src_info)) {}
