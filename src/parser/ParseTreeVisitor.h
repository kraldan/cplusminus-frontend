#pragma once

#include <memory>

#include "utils/Context.h"
#include "CPMParser.h"
#include "ast/all_headers.h"
#include "type/TypeManager.h"


/**
 * Visits antlr generated parse tree to create ast.
 *
 * The ast is not semantically checked, doesn't contain
 * implicit conversions and lacks many more things.
 */
class ParserVisitor {
public:

    explicit ParserVisitor(cpm::Context &context) :
            context(context) {};

    ast::node_ptr<ast::TranslationUnit>
    visitTranslationUnit(CPMParser::TranslationUnitContext *ctx);

    ast::node_ptr<ast::Expr> visitLiteral(CPMParser::LiteralContext *ctx);

    ast::node_ptr<ast::Expr> visitPrimaryExpression(CPMParser::PrimaryExpressionContext *ctx);

    ast::node_ptr<ast::Expr> visitPostfixExpression(CPMParser::PostfixExpressionContext *ctx);

    ast::node_ptr<ast::Expr> visitUnaryExpression(CPMParser::UnaryExpressionContext *ctx);

    ast::node_ptr<ast::Expr> visitCastExpression(CPMParser::CastExpressionContext *ctx);

    ast::node_ptr<ast::Expr>
    visitConditionalExpression(CPMParser::ConditionalExpressionContext *ctx);

    ast::node_ptr<ast::Expr> visitAssignmentExpression(CPMParser::AssignmentExpressionContext *ctx);

    ast::AssignOp visitAssignmentOperator(CPMParser::AssignmentOperatorContext *ctx);

    ast::node_ptr<ast::Expr> visitBinaryExpression(CPMParser::BinaryExpressionContext *ctx);

    ast::node_ptr<ast::Expr> visitConstantExpression(CPMParser::ConstantExpressionContext *ctx);

    ast::node_ptr<ast::Expr> visitCommaExpression(CPMParser::CommaExpressionContext *ctx);

    ast::node_ptr<ast::Condition> visitCondition(CPMParser::ConditionContext *ctx);

    cpm::SimpleType *visitTypeSpecifierSeq(CPMParser::TypeSpecifierSeqContext *ctx);

    cpm::Type *visitTheTypeId(CPMParser::TheTypeIdContext *ctx);

    ast::UnaryOp visitUnaryOperator(CPMParser::UnaryOperatorContext *ctx);

    ast::node_ptr<ast::DeclarStmt> visitDeclarationStatement(
            CPMParser::DeclarationStatementContext *ctx);

    ast::node_ptr<ast::ExprStmt> visitExpressionStatement(
            CPMParser::ExpressionStatementContext *ctx
    );

    ast::node_ptr<ast::CompoundStmt>
    visitCompoundStatement(CPMParser::CompoundStatementContext *ctx);

    ast::node_ptr<ast::Stmt> visitStatement(CPMParser::StatementContext *ctx);

    std::vector<ast::node_ptr<ast::Stmt>>
    visitStatementSeq(CPMParser::StatementSeqContext *ctx);

    ast::node_ptr<ast::Stmt> visitJumpStatement(CPMParser::JumpStatementContext *ctx);

    ast::node_ptr<ast::ForInitStmt> visitForInitStatement(CPMParser::ForInitStatementContext *ctx);

    ast::node_ptr<ast::Stmt> visitIterationStatement(CPMParser::IterationStatementContext *ctx);

    ast::node_ptr<ast::Stmt> visitSelectionStatement(CPMParser::SelectionStatementContext *ctx);

    ast::node_ptr<ast::FuncBody> visitFunctionBody(CPMParser::FunctionBodyContext *ctx);

    ast::node_ptr<ast::FuncDef> visitFunctionDefinition(CPMParser::FunctionDefinitionContext *ctx);

    std::string visitSimpleTypeSpecifier(CPMParser::SimpleTypeSpecifierContext *ctx);

    std::string visitTypeSpecifier(CPMParser::TypeSpecifierContext *ctx);

    std::string visitDeclSpecifier(CPMParser::DeclSpecifierContext *ctx);

    cpm::Type *
    visitDeclSpecifierSeq(CPMParser::DeclSpecifierSeqContext *ctx);

    ast::node_ptr<ast::IdExpr> visitUnqualifiedId(CPMParser::UnqualifiedIdContext *ctx);

    ast::node_ptr<ast::IdExpr> visitIdExpression(CPMParser::IdExpressionContext *ctx);

    ast::node_ptr<ast::Decl> visitNoPointerDeclarator(CPMParser::NoPointerDeclaratorContext *ctx,
                                                      cpm::Type *underlying_type);

    /**
     * See if pointer is declared const (true) or not (false).
     * @param ctx
     * @return
     */
    bool visitPointerOperator(CPMParser::PointerOperatorContext *ctx);

    ast::node_ptr<ast::Decl> visitPointerDeclarator(CPMParser::PointerDeclaratorContext *ctx,
                                                    cpm::Type *underlying_type);

    ast::node_ptr<ast::Decl>
    visitDeclarator(CPMParser::DeclaratorContext *ctx, cpm::Type *underlying_type);

    ast::node_ptr<ast::InitDeclarator> visitInitDeclarator(CPMParser::InitDeclaratorContext *ctx,
                                                           cpm::Type *underlying_type);

    std::vector<ast::node_ptr<ast::InitDeclarator>>
    visitInitDeclaratorList(CPMParser::InitDeclaratorListContext *ctx,
                            cpm::Type *underlying_type);

    ast::node_ptr<ast::Expr> visitInitializer(CPMParser::InitializerContext *ctx,
                                              cpm::Type *decl_type);

    ast::node_ptr<ast::Param>
    visitParameterDeclaration(CPMParser::ParameterDeclarationContext *ctx);

    std::vector<ast::node_ptr<ast::Param>>
    visitParameterDeclarationList(CPMParser::ParameterDeclarationListContext *ctx);

    // <params, vararg>
    std::pair<std::vector<ast::node_ptr<ast::Param>>, bool>
    visitParameterDeclarationClause(CPMParser::ParameterDeclarationClauseContext *ctx);

    /**
     * POD for representing parameters and qualifiers.
     */
    struct ParamsAndQuals {
        std::vector<ast::node_ptr<ast::Param>> params;
        bool vararg;
        bool const_;
        // ref
    };

    ParamsAndQuals
    visitParametersAndQualifiers(CPMParser::ParametersAndQualifiersContext *ctx);

    std::vector<ast::node_ptr<ast::Expr>>
    visitExpressionList(CPMParser::ExpressionListContext *ctx);

    ast::node_ptr<ast::SimpleDeclar>
    visitSimpleDeclaration(CPMParser::SimpleDeclarationContext *ctx);

    ast::node_ptr<ast::Declaration>
    visitDeclaration(CPMParser::DeclarationContext *ctx);

    std::vector<ast::node_ptr<ast::Declaration>>
    visitDeclarationseq(CPMParser::DeclarationseqContext *ctx);

    static std::string visitClassName(CPMParser::ClassNameContext *ctx);

    std::string visitClassHeadName(CPMParser::ClassHeadNameContext *ctx);

    ast::ClassKey visitClassKey(CPMParser::ClassKeyContext *ctx);

    ast::node_ptr<ast::MemberDeclarator>
    visitMemberDeclarator(CPMParser::MemberDeclaratorContext *ctx,
                          cpm::Type *underlying_type);

    ast::node_ptr<ast::MemberDeclaratorList>
    visitMemberDeclaratorList(CPMParser::MemberDeclaratorListContext *ctx,
                              cpm::Type *underlying_type);

    ast::node_ptr<ast::MemberDeclaration>
    visitMemberDeclaration(CPMParser::MemberdeclarationContext *ctx);

    ast::node_ptr<ast::MemberSpecification>
    visitMemberSpecification(CPMParser::MemberSpecificationContext *ctx);

    ast::node_ptr<ast::MemberSpecElem> visitMemberSpecElem(CPMParser::MemberSpecElemContext *ctx);

    ast::node_ptr<ast::AccessModifier> visitAccessSpecifier(CPMParser::AccessSpecifierContext *ctx);

    ast::node_ptr<ast::ClassHead> visitClassHead(CPMParser::ClassHeadContext *ctx);

    ast::node_ptr<ast::ClassDef> visitClassDefinition(CPMParser::ClassDefinitionContext *ctx);

private:
    /* Reports an error in the source code. */
    [[noreturn]] void report_error(const std::string &msg,
                                   antlr4::ParserRuleContext *ctx);

    /* Reports an unhandled (unimplemented) case by the parser.
     * Throws an exception. */
    [[noreturn]] void report_unhandled_case(const std::string &err_loc,
                                            antlr4::ParserRuleContext *ctx);

    /* Reports a warning to std out. */
    void warning(const std::string &msg, antlr4::ParserRuleContext *ctx);

    /**
     * Converts user string from user to string with escape sequences.
     *
     * e.g. if the elements of a string are '\', 'n', return '\n' as a string
     * @param orig string to be unescaped
     * @param ctx ctx for error reporting
     * @return
     */
    std::string unescapeStr(const std::string &orig, antlr4::ParserRuleContext *ctx);

    /**
     * Converts antlr4 Token to ast::BinaryOp.
     * @param op
     * @return
     */
    static ast::BinaryOp binary_op(antlr4::Token *op);

    /**
     * Creates ast::SourceInfo out of 'ctx'.
     * @param ctx
     * @return
     */
    static ast::SourceInfo src_info(antlr4::ParserRuleContext *ctx);

    cpm::Context &context;

    cpm::SimpleType *
    getTypeFromSeq(const std::vector<std::string> &specs, antlr4::ParserRuleContext *ctx);

    bool valid_array_elem_type(cpm::Type *type);

    static bool is_void(cpm::Type *type);


};

