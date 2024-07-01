#include "AstDumper.h"

using namespace std;

void AstDumper::dump_shared(const ast::Node &node, const std::string &node_name,
                            const std::string &info) {
    string line_no = to_string(node.src_info.line_no);
    string col_no = to_string(node.src_info.col_no);
    (*os) << prefix << node_name << " " <<
          "<" << "line:" << node.src_info.str() << ">" << " " <<
          info << endl;
}

void AstDumper::dump_shared(const std::string &node_name, const std::string &info) {
    (*os) << prefix << node_name << "<" << "line: " << "unknown" << ">" << " " << info << endl;
}

void AstDumper::operator()(const ast::IfStmt &node) {
    dump_shared(node, "IfStmt");

    dump_child(*node.cond);
    dump_child(*node.body, !node.else_body.has_value());
    if (node.else_body.has_value())
        dump_child(*node.else_body.value(), true);
}

void AstDumper::operator()(const ast::ForStmt &node) {
    dump_shared(node, "ForStmt");

    dump_child(*node.initStmt);
    if (node.cond)
        dump_child(*node.cond.value());
    if (node.post_iter)
        dump_child(*node.post_iter.value());

    dump_child(*node.body, true);
}

void AstDumper::operator()(const ast::IntLiteral &node) {
    dump_shared(node, "IntLiteral", to_string(node.val));
}

void AstDumper::operator()(const ast::IdExpr &node) {
    string info = node.id;
    if (node.var.has_value())
        info += ", declared on line " + to_string(node.var.value()->src_info.line_no);
    dump_shared(node, "IdExpr", info);
}

void AstDumper::operator()(const ast::CharLiteral &node) {
    dump_shared(node, "CharLiteral", "'"s + node.c + "'"s);
}

void AstDumper::operator()(const ast::ThisExpr &node) {
    dump_shared(node, "ThisExpr");
}

void AstDumper::operator()(const ast::BoolLiteral &node) {
    dump_shared(node, "BoolLiteral", node.val ? "true" : "false");
}

void AstDumper::operator()(const ast::FloatLiteral &node) {
    dump_shared(node, "FloatLiteral", to_string(node.val));
}

void AstDumper::operator()(const ast::NullptrLiteral &node) {
    dump_shared(node, "NullptrLiteral");
}

void AstDumper::operator()(const ast::StringLiteral &node) {
    dump_shared(node, "StringLiteral", "\"" + node.str + "\"");
}

void AstDumper::operator()(const ast::BinaryExpr &node) {
    dump_shared(node, "BinaryExpr", quote(ast::op_to_str(node.op)));

    dump_child(*node.lhs);
    dump_child(*node.rhs, true);
}

void AstDumper::operator()(const ast::AssignmentExpr &node) {
    string op = quote(ast::op_to_str(node.op));
    string lhs = node.lhs_type.has_value() ? "lhs_type=" + cpm::to_string(node.lhs_type.value())
                                           : "";
    string info = op + " " + lhs;
    dump_shared(node, "AssignmentExpr", info);

    dump_child(*node.lhs);
    dump_child(*node.rhs, true);
}

void AstDumper::operator()(const ast::CommaExpr &node) {
    assert(!node.expressions.empty());
    dump_shared(node, "CommaExpr");

    for (size_t i = 0; i < node.expressions.size() - 1; i++)
        dump_child(*node.expressions[i]);

    dump_child(*node.expressions.back());
}

void AstDumper::operator()(const ast::CallExpr &node) {
    string info = node.ctor_call ? "ctor call " : "";
    if (node.func.has_value())
        info += cpm::to_string(node.func.value()->type) + ", function declared on line: " +
                to_string(node.func.value()->src_info.line_no);
    dump_shared(node, "CallExpr", info);

    dump_child(*node.called_func, node.args.empty());

    if (!node.args.empty()) {
        for (size_t i = 0; i < node.args.size() - 1; i++)
            dump_child(*node.args[i]);
        dump_child(*node.args.back(), true);
    }
}

void AstDumper::operator()(const ast::SubscriptExpr &node) {
    dump_shared(node, "SubscriptExpr");

    dump_child(*node.dest);

    dump_child(*node.index, true);
}

void AstDumper::operator()(const ast::TernaryExpr &node) {
    dump_shared(node, "TernaryExpr");
    dump_child(*node.cond);
    dump_child(*node.then);
    dump_child(*node.else_, true);
}

void AstDumper::operator()(const ast::PostIncrExpr &node) {
    dump_shared(node, "PostIncrExpr", quote(node.incr ? string("++") : "--"));
    dump_child(*node.expr, true);
}

void AstDumper::operator()(const ast::UnaryExpr &node) {
    dump_shared(node, "UnaryExpr", quote(ast::op_to_str(node.op)));
    dump_child(*node.expr, true);
}

void AstDumper::operator()(const ast::MemberAccessExpr &node) {
    string info = (node.ptr_access ? "->" : ".") + node.member;
    dump_shared(node, "MemberAccessExpr", info);
    dump_child(*node.object, true);
}

void AstDumper::operator()(const ast::ImplicitTypeCastExpr &node) {
    assert(node.dest_ty);
    dump_shared(node, "ImplicitTypeCastExpr", cpm::to_string(node.dest_ty));
    dump_child(*node.val, true);
}

void AstDumper::operator()(const ast::LValToRValExpr &node) {
    dump_shared(node, "LValToRValExpr");
    dump_child(*node.val, true);
}

void AstDumper::operator()(const ast::DefaultArgExpr &node) {
    dump_shared(node, "DefaultArgExpr");
    dump_child(*node.expr, true);
}

void AstDumper::operator()(const ast::Condition &node) {
    dump_shared(node, "Condition");
    dump_child(*node.expr, true);
}

void AstDumper::operator()(const ast::DeclarStmt &node) {
    dump_shared(node, "DeclarStmt");
    dump_child(*node.declaration, true);
}

void AstDumper::operator()(const ast::ExprStmt &node) {
    dump_shared(node, "ExprStmt");
    if (node.expr.has_value()) {
        dump_child(*node.expr.value(), true);
    }
}

void AstDumper::operator()(const ast::BreakStmt &node) {
    dump_shared(node, "BreakStmt", "break level: " + to_string(node.break_level));
}

void AstDumper::operator()(const ast::ContinueStmt &node) {
    string info =
            node.continue_level == 1 ? "" : "continue level: " + to_string(node.continue_level);
    dump_shared(node, "ContinueStmt", info);
}

void AstDumper::operator()(const ast::ReturnStmt &node) {
    dump_shared(node, "ReturnStmt");
    if (node.expr)
        dump_child(*node.expr.value(), true);
}

void AstDumper::operator()(const ast::CompoundStmt &node) {
    dump_shared(node, "CompoundStmt");
    dump_vector(node.statements);
}

void AstDumper::operator()(const ast::DoWhileStmt &node) {
    dump_shared(node, "DoWhileStmt");
    dump_child(*node.cond);
    dump_child(*node.body, true);
}

void AstDumper::operator()(const ast::WhileStmt &node) {
    dump_shared(node, "WhileStmt");
    dump_child(*node.cond);
    dump_child(*node.body, true);
}

void AstDumper::operator()(const ast::SimpleDeclar &node) {
    dump_shared(node, "SimpleDeclar");
    if (!node.init_declars.empty()) {
        for (size_t i = 0; i < node.init_declars.size() - 1; i++)
            dump_child(*node.init_declars[i]);
        dump_child(*node.init_declars.back(), true);
    }
}

void AstDumper::operator()(const ast::FuncDef &node) {
    dump_shared(node, "FuncDef");
    dump_child(*node.declarator);
    dump_child(*node.body, true);
}

void AstDumper::operator()(const ast::EmptyDeclaration &node) {
    dump_shared(node, "EmptyDeclaration");
}

void AstDumper::operator()(const ast::InitDeclarator &node) {
    dump_shared(node, "InitDeclarator");
    dump_child(*node.declarator, !node.initializer.has_value());
    if (node.initializer.has_value())
        dump_child(*node.initializer.value(), true);
}

void AstDumper::operator()(const ast::FuncBody &node) {
    dump_shared(node, "FuncBody");
    dump_child(*node.comp_stmt, true);
}

void AstDumper::operator()(const ast::Param &node) {
    dump_shared(node, "Param");
    dump_child(*node.declarator, !node.default_val.has_value());
    if (node.default_val.has_value()) {
        dump_child(*node.default_val.value(), true);
    }
}

void AstDumper::operator()(const ast::TranslationUnit &node) {
    dump_shared(node, "TranslationUnit");
    dump_vector(node.declars);
}

void AstDumper::addPrefix(bool slash) {
    if (!prefix.empty()) {
        prefix.pop_back();
        prefix.push_back(' ');
    }
    prefix.push_back(slash ? '|' : ' ');
    prefix.push_back('-');
}

void AstDumper::dropPrefix() {
    prefix.pop_back();
    prefix.pop_back();
    if (!prefix.empty()) {
        prefix.pop_back();
        prefix.push_back('-');
    }
}

template<typename T>
void AstDumper::dump_vector(const vector<ast::node_ptr<T>> &v) {
    if (v.empty())
        return;
    for (size_t i = 0; i < v.size() - 1; i++)
        dump_child(*v[i]);
    dump_child(*v.back(), true);
}

void AstDumper::operator()(const ast::ClassDef &node) {
    dump_shared(node, "ClassDef",
                (node.head->key == ast::Struct ? "struct " : "class ") + quote(node.head->name));
    if (node.body)
        dump_child(*node.body.value(), true);
}

void AstDumper::operator()(const ast::MemberSpecification &node) {
    dump_shared(node, "MemberSpecification");
    dump_vector(node.list);
}

void AstDumper::operator()(const ast::MemberSpecElem &node) {
    std::visit(*this, node);
}

void AstDumper::operator()(const ast::MemberDeclaration &node) {
    std::visit(*this, node);
}

void AstDumper::operator()(const ast::AccessModifier &node) {
    dump_shared("AccessModifier", node == ast::PUBLIC ? "public" : "private");
}

void AstDumper::operator()(const ast::MemberDeclaratorList &node) {
    dump_shared(node, "MemberDeclaratorList");
    dump_vector(node.decls);
}

void AstDumper::operator()(const ast::ImplicitThisExpr &node) {
    dump_shared(node, "ImplicitThisExpr");
}

void AstDumper::operator()(const ast::ArrToPtrExpr &node) {
    dump_shared(node, "ArrToPtrExpr");
    dump_child(*node.arr_expr, true);
}

std::string AstDumper::quote(const string &s) {
    return "'" + s + "'";
}

template<typename T>
std::string AstDumper::quote(T num) {
    return quote(to_string(num));
}

template<typename T>
void AstDumper::dump_child(const T &node, bool is_last) {
    addPrefix(!is_last);
    dump(node);
    dropPrefix();
}

void AstDumper::operator()(const ast::CastExpr &node) {
    dump_shared(node, "CastExpr", cpm::to_string(node.type));
    dump_child(*node.expr, true);
}

void AstDumper::operator()(const ast::SizeofTypeExpr &node) {
    assert(node.type);
    dump_shared(node, "SizeofTypeExpr", cpm::to_string(node.type));
}

void AstDumper::operator()(const ast::Decl &node) {
    const auto *func_decl = dynamic_cast<const ast::FunctionDecl *>(&node);
    if (func_decl)
        dump(*func_decl);
    else
        dump_shared(node, "Decl"s, node.id + " " + cpm::to_string(node.type));
}

void AstDumper::operator()(const ast::FunctionDecl &node) {
    string info = node.id + " " + cpm::to_string(node.type);
    if (node.orig.has_value())
        info += ", first declaration: line " + to_string(node.orig.value()->src_info.line_no);
    dump_shared(node, "FunctionDecl"s, info);
    const auto &params = node.params;
    for (size_t i = 0; i < params.size(); i++)
        dump_child(*params[i], i == params.size() - 1);
}




