#pragma once


#include "ast/all_headers.h"

/* Template with predeclared () operators. It might not cover all ast nodes (some forgotten,
 * some new), but it should be the bulk of the work. */
class Visitor {
public:

    /* aliases for return types */
    using ExprRet = void;
    using StmtRet = void;
    using ForInitStmtRet = void;
    using DeclSpecRet = void;
    using NPDRet = void;
    using DeclarationRet = void;
    // this is needed because of ast::ForInitStmt
    static_assert(std::is_same<DeclarationRet, StmtRet>::value);

    /* expressions */
    ExprRet operator()(const ast::Expr &node) {
        return std::visit(*this, node);
    }

    ExprRet operator()(const ast::IntLiteral &node);

    ExprRet operator()(const ast::IdExpr &node);

    ExprRet operator()(const ast::CharLiteral &node);

    ExprRet operator()(const ast::ThisExpr &node);

    ExprRet operator()(const ast::BoolLiteral &node);

    ExprRet operator()(const ast::FloatLiteral &node);

    ExprRet operator()(const ast::Nullptr &node);

    ExprRet operator()(const ast::StringLiteral &node);

    ExprRet operator()(const ast::SizeofType &node);

    ExprRet operator()(const ast::BinaryExpr &node);

    ExprRet operator()(const ast::AssignmentExpr &node);

    ExprRet operator()(const ast::CommaExpr &node);

    ExprRet operator()(const ast::CallExpr &node);

    ExprRet operator()(const ast::SubscriptExpr &node);

    ExprRet operator()(const ast::TernaryExpr &node);

    ExprRet operator()(const ast::PostIncrExpr &node);

    ExprRet operator()(const ast::UnaryExpr &node);

    ExprRet operator()(const ast::CastExpr &node);

    ExprRet operator()(const ast::MemberAccess &node);

    /* statements */
    StmtRet operator()(const ast::Stmt &node) {
        return std::visit(*this, node);
    }

    StmtRet operator()(const ast::DeclarStmt &node);

    StmtRet operator()(const ast::ExprStmt &node);

    StmtRet operator()(const ast::BreakStmt &node);

    StmtRet operator()(const ast::ContinueStmt &node);

    StmtRet operator()(const ast::ReturnStmt &node);

    StmtRet operator()(const ast::CompoundStmt &node);

    StmtRet operator()(const ast::DoWhileStmt &node);

    StmtRet operator()(const ast::ForStmt &node);

    StmtRet operator()(const ast::IfStmt &node);

    StmtRet operator()(const ast::WhileStmt &node);

    /* declarations */
    DeclarationRet operator()(const ast::Declaration &node) {
        return std::visit(*this, node);
    }

    DeclarationRet operator()(const ast::SimpleDeclar &node);

    DeclarationRet operator()(const ast::FuncDef &node);

    DeclarationRet operator()(const ast::ClassDef &node);

    DeclarationRet operator()(const ast::EmptyDeclaration &node);

    /* for loop init statements */
    ForInitStmtRet operator()(const ast::ForInitStmt &node) {
        return std::visit(*this, node);
    }
    // both already declared above
//    ForInitStmtRet operator()(const ast::ExprStmt &node);
//    DeclarationRet operator()(const ast::SimpleDeclar &node);

    /* no-pointer declarators */
    NPDRet operator()(const ast::NoPointerDeclarator &node) {
        return std::visit(*this, node);
    }

    NPDRet operator()(const ast::DeclID &node);

    NPDRet operator()(const ast::FunctionNPD &node);

    NPDRet operator()(const ast::ArrayNPD &node);

    NPDRet operator()(const ast::PointerDeclarator &node);

    /* declaration specifiers */
    DeclSpecRet operator()(const ast::DeclSpecifier &node) {
        return std::visit(*this, node);
    }

    DeclSpecRet operator()(const ast::TypeSpec &node);


    /* the rest that are not part of a variant */
    void operator()(const ast::DeclSpecifierSeq &node);

    void operator()(const ast::InitDeclarator &node);

    void operator()(const ast::TypeSpecSeq &node);

    void operator()(const ast::FuncBody &node);

    void operator()(const ast::Param &node);

    void operator()(const ast::ParamsAndQuals &node);

    void operator()(const ast::TranslationUnit &node);
};