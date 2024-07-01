#pragma once

#include <string>
#include <iostream>

#include "ast/all_headers.h"
#include "type/DerivedTypes.h"

/**
 * Dumps asts.
 *
 * Output format was largely inspired by 'clang -Xclang -ast-dump'.
 */
class AstDumper {
public:
    /**
     * Dump an AST node to given os.
     */
    template<typename T>
    void run(const T &node, std::ostream &os) {
        this->os = &os;
        dump(node);
    }

    /* the following () methods are public because std::visit requires it, don't use
     * them from the outside */

    /* expressions */
    void operator()(const ast::Expr &node) {
        return std::visit(*this, node);
    }

    void operator()(const ast::IntLiteral &node);

    void operator()(const ast::IdExpr &node);

    void operator()(const ast::CharLiteral &node);

    void operator()(const ast::ThisExpr &node);

    void operator()(const ast::BoolLiteral &node);

    void operator()(const ast::FloatLiteral &node);

    void operator()(const ast::NullptrLiteral &node);

    void operator()(const ast::StringLiteral &node);

    void operator()(const ast::BinaryExpr &node);

    void operator()(const ast::AssignmentExpr &node);

    void operator()(const ast::CommaExpr &node);

    void operator()(const ast::CallExpr &node);

    void operator()(const ast::SubscriptExpr &node);

    void operator()(const ast::TernaryExpr &node);

    void operator()(const ast::PostIncrExpr &node);

    void operator()(const ast::UnaryExpr &node);

    void operator()(const ast::CastExpr &node);

    void operator()(const ast::MemberAccessExpr &node);

    void operator()(const ast::ImplicitTypeCastExpr &node);

    void operator()(const ast::LValToRValExpr &node);

    void operator()(const ast::DefaultArgExpr &node);

    void operator()(const ast::ImplicitThisExpr &node);

    void operator()(const ast::ArrToPtrExpr &node);

    void operator()(const ast::SizeofTypeExpr &node);

    void operator()(const ast::Condition &node);

    /* statements */
    void operator()(const ast::Stmt &node) {
        return std::visit(*this, node);
    }

    void operator()(const ast::DeclarStmt &node);

    void operator()(const ast::ExprStmt &node);

    void operator()(const ast::BreakStmt &node);

    void operator()(const ast::ContinueStmt &node);

    void operator()(const ast::ReturnStmt &node);

    void operator()(const ast::CompoundStmt &node);

    void operator()(const ast::DoWhileStmt &node);

    void operator()(const ast::ForStmt &node);

    void operator()(const ast::IfStmt &node);

    void operator()(const ast::WhileStmt &node);

    /* declarations */
    void operator()(const ast::Declaration &node) {
        return std::visit(*this, node);
    }

    void operator()(const ast::SimpleDeclar &node);

    void operator()(const ast::FuncDef &node);

    void operator()(const ast::ClassDef &node);

    void operator()(const ast::MemberSpecification &node);

    void operator()(const ast::MemberSpecElem &node);

    void operator()(const ast::MemberDeclaration &node);

    void operator()(const ast::AccessModifier &node);

    void operator()(const ast::MemberDeclaratorList &node);

    void operator()(const ast::EmptyDeclaration &node);

    /* for loop init statements */
    void operator()(const ast::ForInitStmt &node) {
        return std::visit(*this, node);
    }
    /* declaration specifiers */

    /* the rest that are not part of a variant */
    void operator()(const ast::InitDeclarator &node);

    void operator()(const ast::FuncBody &node);

    void operator()(const ast::Param &node);

    void operator()(const ast::TranslationUnit &node);

    void operator()(const ast::Decl &node);

    void operator()(const ast::FunctionDecl &node);

private:
    std::string prefix = "";

    std::ostream *os;

    // syntactic sugar
    AstDumper &dump = *this;

    /**
 * Shared work for all ast nodes.
 */
    void
    dump_shared(const ast::Node &node, const std::string &node_name, const std::string &info = "");

    /**
     * Replacement for 'nodes' that are not Node, but an enum or something.
     */
    void dump_shared(const std::string &node_name, const std::string &info = "");

    /**
     * Increases the prefix level to go one level deeper.
     * @param slash Whether new records should start with a slash '|' or not.
     */
    void addPrefix(bool slash);

    /**
     * Drops one prefix level.
     */
    void dropPrefix();

    /**
    * Shortcut for nodes that a contain child nodes in a vector, that is the last member of the node.
    * @tparam T
    * @param v
    */
    template<typename T>
    void dump_vector(const std::vector<ast::node_ptr<T>> &v);


    /**
     * Dump a child node with extra indentation.
     * @tparam T
     * @param slash
     */
    template<typename T>
    void dump_child(const T &, bool is_last = false);

    /**
     * Add single quotes around a string.
     * @param s
     * @return
     */
    static std::string quote(const std::string &s);

    template<typename T>
    static std::string quote(T num);
};


