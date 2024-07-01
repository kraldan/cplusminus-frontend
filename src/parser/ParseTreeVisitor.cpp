#include <ostream>

#include "ParseTreeVisitor.h"

using namespace std;

using namespace ast;


std::string ParserVisitor::unescapeStr(const std::string &orig, antlr4::ParserRuleContext *ctx) {
    std::string res;
    bool escape = false;
    for (const auto &c: orig) {
        if (escape) {
            switch (c) {
                case 'n':
                    res.push_back('\n');
                    break;
                case 't':
                    res.push_back('\t');
                    break;
                case '0':
                    res.push_back('\0');
                    break;
                case '\\':
                    res.push_back('\\');
                    break;
                default:
                    warning("unknown escape sequence \'\\" + to_string(c) + "\'", ctx);
                    res += "\\" + to_string(c);
            }
            escape = false;
        } else {
            if (c == '\\')
                escape = true;
            else
                res.push_back(c);
        }
    }
    return res;
}

ast::node_ptr<ast::Expr> ParserVisitor::visitLiteral(CPMParser::LiteralContext *ctx) {
    auto source_info = src_info(ctx);
    if (auto il = ctx->IntegerLiteral()) {
        uint64_t val = std::stoi(il->getText());
        return make_node<IntLiteral, Expr>(std::move(source_info), val);
    } else if (auto cl = ctx->CharacterLiteral()) {
        // the format is 'c'
        string text = cl->getText().substr(1, cl->getText().length() - 2);
        string unescaped_text = unescapeStr(text, ctx);
        if (unescaped_text.length() != 1)
            report_error("not a char literal", ctx);
        char c = unescaped_text[0];
        return make_node<CharLiteral, Expr>(std::move(source_info), c);
    } else if (auto fl = ctx->FloatingLiteral()) {
        double val = std::stod(fl->getText());
        return make_node<FloatLiteral, Expr>(std::move(source_info),
                                             val);
    } else if (auto sl = ctx->StringLiteral()) {
        size_t len = sl->getText().size();
        // 1, len-2 for quotes around the string
        std::string str = sl->getText().substr(1, len - 2);
        str = unescapeStr(str, ctx);
        return make_node<StringLiteral, Expr>(std::move(source_info),
                                              std::move(str));
    } else if (auto bl = ctx->BooleanLiteral()) {
        bool val = bl->getText() == "true";
        return make_node<BoolLiteral, Expr>(std::move(source_info),
                                            val);
    } else if (ctx->PointerLiteral())
        return make_node<NullptrLiteral, Expr>(std::move(source_info));
    report_unhandled_case("visitLiteral", ctx);
}

ast::node_ptr<ast::Expr>
ParserVisitor::visitPrimaryExpression(CPMParser::PrimaryExpressionContext *ctx) {
    auto source_info = src_info(ctx);
    auto literals = ctx->literal();

    if (not literals.empty()) {
        // multiple strings
        if (literals.size() > 1) {
            string final_str;
            for (const auto &l: literals) {
                auto expr = visitLiteral(l);
                if (auto str_l = get_if<StringLiteral>(expr.get()))
                    final_str += str_l->str;
                else
                    report_error("got multiple literals, but not all are string literals", ctx);
            }
            /* when SourceInfo is more developed,
             * maybe it should be taken here from the first literal,
             * or some mix from all literals, not from PrimaryExpressionCtx? */
            return make_node<StringLiteral, Expr>(std::move(source_info), std::move(final_str));
        } else
            return visitLiteral(literals[0]);
    } else if (ctx->This())
        return make_node<ThisExpr, Expr>(std::move(source_info));
    else if (ctx->LeftParen())
        return visitCommaExpression(ctx->commaExpression());
    else if (auto child = ctx->idExpression()) {
        return change_node<IdExpr, Expr>(visitIdExpression(child));
    }
    report_unhandled_case("visitPrimaryExpression", ctx);
}

ast::node_ptr<ast::Expr>
ParserVisitor::visitPostfixExpression(CPMParser::PostfixExpressionContext *ctx) {
    auto source_info = src_info(ctx);
    if (auto pec = ctx->primaryExpression()) {
        return visitPrimaryExpression(pec);
    }
        // subscript
    else if (ctx->LeftBracket()) {
        if (ctx->commaExpression()) {
            node_ptr<Expr> index = visitCommaExpression(ctx->commaExpression());
            return make_node<SubscriptExpr, Expr>(std::move(source_info),
                                                  visitPostfixExpression(ctx->postfixExpression()),
                                                  std::move(index));
        }
    }
        // call
    else if (ctx->postfixExpression() && ctx->LeftParen() && ctx->RightParen()) {
        auto callee = visitPostfixExpression(ctx->postfixExpression());
        vector<node_ptr<Expr>> args;
        if (ctx->expressionList()) args = visitExpressionList(ctx->expressionList());
        return make_node<CallExpr, Expr>(std::move(source_info),
                                         std::move(callee),
                                         std::move(args));
    }
        // post incr/decr
    else if (ctx->postfixExpression() &&
             (ctx->PlusPlus() || ctx->MinusMinus()))
        return make_node<PostIncrExpr, Expr>(std::move(source_info),
                                             visitPostfixExpression(ctx->postfixExpression()),
                                             ctx->PlusPlus());
    else if (ctx->postfixExpression() && (ctx->Dot() || ctx->Arrow()) &&
             ctx->idExpression()) {
        auto object = visitPostfixExpression(ctx->postfixExpression());
        bool ptr_access = ctx->Arrow();
        auto member = visitIdExpression(ctx->idExpression());
        return make_node<MemberAccessExpr, Expr>(std::move(source_info),
                                                 std::move(object), ptr_access, member->id);
    }
    report_unhandled_case("visitPostfixExpression", ctx);
}

ast::node_ptr<ast::Expr>
ParserVisitor::visitUnaryExpression(CPMParser::UnaryExpressionContext *ctx) {
    auto source_info = src_info(ctx);
    if (auto pec = ctx->postfixExpression()) {
        return visitPostfixExpression(pec);
    } else if (ctx->unaryOperator() && ctx->castExpression())
        return make_node<UnaryExpr, Expr>(std::move(source_info),
                                          visitUnaryOperator(ctx->unaryOperator()),
                                          visitCastExpression(ctx->castExpression()));
    else if (ctx->Sizeof() && ctx->theTypeId())
        return make_node<SizeofTypeExpr, Expr>(std::move(source_info),
                                               visitTheTypeId(ctx->theTypeId()));
    else
        report_unhandled_case("visitUnaryExpression", ctx);
}

ast::node_ptr<ast::Expr> ParserVisitor::visitCastExpression(CPMParser::CastExpressionContext *ctx) {
    auto source_info = src_info(ctx);
    if (auto uec = ctx->unaryExpression()) {
        return visitUnaryExpression(uec);
    } else if (ctx->theTypeId() && ctx->castExpression()) {
        cpm::Type *type = visitTheTypeId(ctx->theTypeId());
        return make_node<CastExpr, Expr>(std::move(source_info),
                                         type,
                                         visitCastExpression(ctx->castExpression()));
    } else
        report_unhandled_case("visitCastExpression", ctx);
}

BinaryOp ParserVisitor::binary_op(antlr4::Token *op) {
    switch (op->getType()) {
        case CPMParser::Star:
            return ast::Star;
        case CPMParser::Div:
            return ast::Div;
        case CPMParser::Mod:
            return ast::Mod;
        case CPMParser::Plus:
            return ast::Plus;
        case CPMParser::Minus:
            return ast::Minus;
        case CPMParser::Less:
            return ast::Less;
        case CPMParser::Greater:
            return ast::Greater;
        case CPMParser::LessEqual:
            return ast::LessEqual;
        case CPMParser::GreaterEqual:
            return ast::GreaterEqual;
        case CPMParser::Equal:
            return ast::Equal;
        case CPMParser::NotEqual:
            return ast::NotEqual;
        case CPMParser::And:
            return ast::And;
        case CPMParser::Caret:
            return ast::Caret;
        case CPMParser::Or:
            return ast::Or;
        case CPMParser::AndAnd:
            return ast::LogicalAnd;
        case CPMParser::OrOr:
            return ast::LogicalOr;
        default:
            assert(false && "binary_op: unhandled case");
    }
}

ast::node_ptr<ast::Expr>
ParserVisitor::visitBinaryExpression(CPMParser::BinaryExpressionContext *ctx) {
    auto source_info = src_info(ctx);
    if (ctx->castExpression())
        return visitCastExpression(ctx->castExpression());
    BinaryOp op = ctx->op ? binary_op(ctx->op) : ctx->gr ? BinaryOp::RightShift
                                                         : BinaryOp::LeftShift;
    auto left = visitBinaryExpression(ctx->lhs);
    auto right = visitBinaryExpression(ctx->rhs);
    return make_node<BinaryExpr, Expr>(std::move(source_info),
                                       std::move(left), std::move(right), op);
}

ast::node_ptr<ast::Expr>
ParserVisitor::visitConstantExpression(CPMParser::ConstantExpressionContext *ctx) {
//    auto source_info = src_info(ctx);
    return visitConditionalExpression(ctx->conditionalExpression());
}

std::string
ParserVisitor::visitSimpleTypeSpecifier(CPMParser::SimpleTypeSpecifierContext *ctx) {
    auto source_info = src_info(ctx);

    if (ctx->Int())
        return "int"s;
    else if (ctx->Char())
        return "char"s;
    else if (ctx->Bool())
        return "bool"s;
    else if (ctx->Double())
        return "double"s;
    else if (ctx->Void())
        return "void"s;
    else if (ctx->theTypeName())
        return visitClassName(ctx->theTypeName()->className());
    report_unhandled_case("visitSimpleTypeSpecifier", ctx);
}

std::string ParserVisitor::visitTypeSpecifier(CPMParser::TypeSpecifierContext *ctx) {
    auto source_info = src_info(ctx);
    if (auto sts = ctx->simpleTypeSpecifier()) {
        return visitSimpleTypeSpecifier(sts);
    } else if (ctx->Const())
        return "const";
    else
        report_unhandled_case("visitTypeSpecifier", ctx);
}

std::string ParserVisitor::visitDeclSpecifier(CPMParser::DeclSpecifierContext *ctx) {
//    auto source_info = src_info(ctx);
    if (auto child = ctx->typeSpecifier())
        return visitTypeSpecifier(child);
    else
        report_unhandled_case("visitDeclSpecifier", ctx);
}

cpm::Type *
ParserVisitor::visitDeclSpecifierSeq(CPMParser::DeclSpecifierSeqContext *ctx) {
    auto source_info = src_info(ctx);
    vector<string> decl_specs;
    for (const auto &child: ctx->declSpecifier())
        decl_specs.push_back(visitDeclSpecifier(child));
    return getTypeFromSeq(decl_specs, ctx);
}

ast::node_ptr<ast::IdExpr> ParserVisitor::visitUnqualifiedId(CPMParser::UnqualifiedIdContext *ctx) {
    auto source_info = src_info(ctx);
    return make_node<IdExpr>(std::move(source_info), ctx->getText());
}

ast::node_ptr<ast::IdExpr> ParserVisitor::visitIdExpression(CPMParser::IdExpressionContext *ctx) {
//    auto source_info = src_info(ctx);
    if (auto child = ctx->unqualifiedId())
        return visitUnqualifiedId(child);
    else {
        report_unhandled_case("visitIdExpression", ctx);
    }
}

ast::node_ptr<ast::Decl>
ParserVisitor::visitNoPointerDeclarator(CPMParser::NoPointerDeclaratorContext *ctx,
                                        cpm::Type *underlying_type) {
    auto source_info = src_info(ctx);
    // declarator id
    if (auto child = ctx->declaratorID()) {
        return make_node<Decl>(std::move(source_info), underlying_type,
                               visitIdExpression(child->idExpression())->id);
    }
        // param npd
    else if (ctx->noPointerDeclarator() && ctx->parametersAndQualifiers()) {
        ParamsAndQuals paq = visitParametersAndQualifiers(ctx->parametersAndQualifiers());
        vector<cpm::Type *> param_types;
        for (const auto &p: paq.params)
            param_types.push_back(p->declarator->type);
        cpm::FunctionType *ft = context.getFunctionType(underlying_type, param_types, paq.vararg);
        if (paq.const_)
            report_error("const methods are not implemented", ctx);

        node_ptr<Decl> decl = visitNoPointerDeclarator(ctx->noPointerDeclarator(), ft);
        if (decl->type != ft)
            //  currently, there's no "function" checking for function pointers etc.,
            //  such as forbidding default arguments, repeated parameter names etc.
            //  currently this is handled by semantic checker, which doesn't allow anything
            //  but functions
            return decl;

        return make_node<FunctionDecl>(std::move(source_info),
                                       ft,
                                       std::move(decl->id),
                                       std::move(paq.params));
    }
        // array npd
    else if (ctx->noPointerDeclarator() && ctx->LeftBracket() && ctx->RightBracket()) {
        optional<size_t> size;
        if (ctx->constantExpression()) {
            // refactor: rewrite grammar to contain strictly a integer literal
            node_ptr<Expr> size_expr = visitConstantExpression(ctx->constantExpression());
            if (auto il = get_if<IntLiteral>(size_expr.get()))
                size = il->val;
            else
                report_error("array size something else than integer literal", ctx);
        }
        cpm::ArrayType *at = context.getArrayType(underlying_type, size);
        return visitNoPointerDeclarator(ctx->noPointerDeclarator(), at);
    }
    // pointer declarator
    else if (ctx->pointerDeclarator() && ctx->LeftParen() && ctx->RightParen()) {
        return visitPointerDeclarator(ctx->pointerDeclarator(), underlying_type);
    } else
        report_unhandled_case("noPointerDeclarator", ctx);
}

bool ParserVisitor::visitPointerOperator(CPMParser::PointerOperatorContext *ctx) {
//    auto source_info = src_info(ctx);
    return ctx->Const();
}

ast::node_ptr<ast::Decl> ParserVisitor::visitPointerDeclarator(
        CPMParser::PointerDeclaratorContext *ctx,
        cpm::Type *underlying_type) {
    auto source_info = src_info(ctx);
    cpm::Type *t = underlying_type;
    std::vector<bool> pointers;
    for (const auto &po: ctx->pointerOperator()) {
        bool const_ptr = visitPointerOperator(po);
        t = context.getPointerType(t, const_ptr);
    }
    return visitNoPointerDeclarator(ctx->noPointerDeclarator(), t);
}

ast::node_ptr<ast::Decl> ParserVisitor::visitDeclarator(CPMParser::DeclaratorContext *ctx,
                                                        cpm::Type *underlying_type) {
    return visitPointerDeclarator(ctx->pointerDeclarator(), underlying_type);
}

ast::node_ptr<ast::InitDeclarator> ParserVisitor::visitInitDeclarator(
        CPMParser::InitDeclaratorContext *ctx,
        cpm::Type *underlying_type) {
    auto source_info = src_info(ctx);
    auto declarator = visitDeclarator(ctx->declarator(), underlying_type);
    optional<node_ptr<Expr>> initializer = std::nullopt;
    if (auto child = ctx->initializer())
        initializer = visitInitializer(child, declarator->type);
    return make_node<InitDeclarator>(std::move(source_info), std::move(declarator),
                                     std::move(initializer));
}

vector<ast::node_ptr<ast::InitDeclarator>>
ParserVisitor::visitInitDeclaratorList(CPMParser::InitDeclaratorListContext *ctx,
                                       cpm::Type *underlying_type) {
//    auto source_info = src_info(ctx);
    auto res = vector<node_ptr<InitDeclarator>>();
    for (const auto &child: ctx->initDeclarator())
        res.push_back(visitInitDeclarator(child, underlying_type));
    return res;
}

ast::node_ptr<ast::SimpleDeclar>
ParserVisitor::visitSimpleDeclaration(CPMParser::SimpleDeclarationContext *ctx) {
    auto source_info = src_info(ctx);
    cpm::Type *underlying_type = visitDeclSpecifierSeq(ctx->declSpecifierSeq());
    vector<node_ptr<InitDeclarator>> init_declarators;
    if (ctx->initDeclaratorList())
        init_declarators = visitInitDeclaratorList(ctx->initDeclaratorList(), underlying_type);
    return make_node<SimpleDeclar>(std::move(source_info), std::move(init_declarators));
}

ast::node_ptr<ast::Declaration>
ParserVisitor::visitDeclaration(CPMParser::DeclarationContext *ctx) {
    auto source_info = src_info(ctx);
    if (auto child = ctx->simpleDeclaration()) {
        auto sd = visitSimpleDeclaration(child);
        return change_node<SimpleDeclar, Declaration>(std::move(sd));
    } else if (ctx->functionDefinition()) {
        auto fd = visitFunctionDefinition(ctx->functionDefinition());
        return change_node<FuncDef, Declaration>(std::move(fd));
    } else if (ctx->classDefinition()) {
        auto cd = visitClassDefinition(ctx->classDefinition());
        return change_node<ClassDef, Declaration>(std::move(cd));
    } else if (ctx->emptyDeclaration())
        return make_node<EmptyDeclaration, Declaration>(std::move(source_info));
    report_unhandled_case("visitDeclaration", ctx);
}

std::vector<ast::node_ptr<ast::Declaration>>
ParserVisitor::visitDeclarationseq(CPMParser::DeclarationseqContext *ctx) {
//    auto source_info = src_info(ctx);
    vector<node_ptr<Declaration>> res;
    for (const auto &d: ctx->declaration())
        res.push_back(visitDeclaration(d));
    return res;
}

ast::node_ptr<ast::TranslationUnit>
ParserVisitor::visitTranslationUnit(CPMParser::TranslationUnitContext *ctx) {
    auto source_info = src_info(ctx);
    vector<node_ptr<Declaration>> res;
    if (auto child = ctx->declarationseq())
        res = visitDeclarationseq(child);
    else
        warning("visitTranslationUnit: empty declaration sequence", ctx);
    return make_node<TranslationUnit>(std::move(source_info), std::move(res));
}

ast::node_ptr<ast::Expr>
ParserVisitor::visitConditionalExpression(CPMParser::ConditionalExpressionContext *ctx) {
    auto source_info = src_info(ctx);
    if (ctx->Question()) {
        return make_node<TernaryExpr, Expr>(std::move(source_info),
                                            visitBinaryExpression(ctx->binaryExpression()),
                                            visitCommaExpression(ctx->commaExpression()),
                                            visitAssignmentExpression(ctx->assignmentExpression()));
    } else
        return visitBinaryExpression(ctx->binaryExpression());
}

ast::node_ptr<ast::Expr>
ParserVisitor::visitAssignmentExpression(CPMParser::AssignmentExpressionContext *ctx) {
    auto source_info = src_info(ctx);
    if (ctx->assignmentOperator()) {
        auto lhs = visitBinaryExpression(ctx->binaryExpression());
        auto op = visitAssignmentOperator(ctx->assignmentOperator());
        auto rhs = visitAssignmentExpression(ctx->assignmentExpression());
        return make_node<AssignmentExpr, Expr>(std::move(source_info),
                                               std::move(lhs),
                                               std::move(rhs),
                                               op);
    }
    return visitConditionalExpression(ctx->conditionalExpression());
}

ast::node_ptr<ast::Expr> ParserVisitor::visitInitializer(CPMParser::InitializerContext *ctx,
                                                         cpm::Type *decl_type) {
    auto source_info = src_info(ctx);
    if (ctx->Assign() && ctx->assignmentExpression())
        return visitAssignmentExpression(ctx->assignmentExpression());
    else if (ctx->LeftParen() && ctx->expressionList() && ctx->RightParen()) {
        cpm::SimpleType *simple_ty = cpm::simple_ty(decl_type);
        if (!simple_ty)
            report_error("invalid declarator initializer", ctx);
        // create a call expression to the constructor
        // note: this only works for class types, not fundamental types or pointers
        return ast::make_node<ast::CallExpr, ast::Expr>(
                source_info,
                ast::make_node<ast::IdExpr, ast::Expr>(source_info, simple_ty->getTypeId()),
                visitExpressionList(ctx->expressionList())
        );
    } else
        report_unhandled_case("visitInitializer", ctx);
}

ast::node_ptr<ast::Param>
ParserVisitor::visitParameterDeclaration(CPMParser::ParameterDeclarationContext *ctx) {
    auto source_info = src_info(ctx);
    auto underlying_type = visitDeclSpecifierSeq(ctx->declSpecifierSeq());
    auto declarator = visitDeclarator(ctx->declarator(), underlying_type);
    optional<node_ptr<Expr>> default_val;
    if (ctx->assignmentExpression())
        default_val = visitAssignmentExpression(ctx->assignmentExpression());
    return make_node<Param>(std::move(source_info), std::move(declarator),
                            std::move(default_val));
}

std::vector<ast::node_ptr<ast::Param>>
ParserVisitor::visitParameterDeclarationList(CPMParser::ParameterDeclarationListContext *ctx) {
//    auto source_info = src_info(ctx);
    vector<node_ptr<Param>> res;
    for (const auto &pd: ctx->parameterDeclaration())
        res.push_back(visitParameterDeclaration(pd));
    return res;
}

std::pair<std::vector<ast::node_ptr<ast::Param>>, bool>
ParserVisitor::visitParameterDeclarationClause(CPMParser::ParameterDeclarationClauseContext *ctx) {
//    auto source_info = src_info(ctx);
    auto decl_list = visitParameterDeclarationList(ctx->parameterDeclarationList());
    bool vararg = ctx->Ellipsis();
    return make_pair(std::move(decl_list), vararg);
}

ParserVisitor::ParamsAndQuals
ParserVisitor::visitParametersAndQualifiers(CPMParser::ParametersAndQualifiersContext *ctx) {
    auto source_info = src_info(ctx);
    vector<node_ptr<Param>> params;
    bool vararg{};
    if (ctx->parameterDeclarationClause())
        std::tie(params, vararg) = visitParameterDeclarationClause(
                ctx->parameterDeclarationClause());
    return {std::move(params), vararg, ctx->Const() != nullptr};
}

ast::node_ptr<ast::DeclarStmt>
ParserVisitor::visitDeclarationStatement(CPMParser::DeclarationStatementContext *ctx) {
    auto source_info = src_info(ctx);
    auto bd = visitSimpleDeclaration(ctx->simpleDeclaration());
    return make_node<DeclarStmt>(std::move(source_info), std::move(bd));
}// for now, automatically put void type

ast::node_ptr<ast::ExprStmt>
ParserVisitor::visitExpressionStatement(CPMParser::ExpressionStatementContext *ctx) {
    auto source_info = src_info(ctx);
    optional<node_ptr<Expr>> expr;
    if (ctx->commaExpression())
        expr = visitCommaExpression(ctx->commaExpression());
    return make_node<ExprStmt>(std::move(source_info), std::move(expr));
}

ast::node_ptr<ast::CompoundStmt>
ParserVisitor::visitCompoundStatement(CPMParser::CompoundStatementContext *ctx) {
    auto source_info = src_info(ctx);
    vector<node_ptr<Stmt>> stats;
    if (ctx->statementSeq())
        stats = visitStatementSeq(ctx->statementSeq());
    return make_node<CompoundStmt>(std::move(source_info), std::move(stats));
}

std::vector<ast::node_ptr<ast::Stmt>>
ParserVisitor::visitStatementSeq(CPMParser::StatementSeqContext *ctx) {
//    auto source_info = src_info(ctx);
    vector<node_ptr<Stmt>> res;
    for (const auto &s: ctx->statement())
        res.push_back(visitStatement(s));
    return res;
}

ast::node_ptr<ast::Stmt> ParserVisitor::visitStatement(CPMParser::StatementContext *ctx) {
//    auto source_info = src_info(ctx);
    if (ctx->declarationStatement())
        return change_node<DeclarStmt, Stmt>(
                visitDeclarationStatement(ctx->declarationStatement())
        );
    else if (ctx->expressionStatement())
        return change_node<ExprStmt, Stmt>(
                visitExpressionStatement(ctx->expressionStatement())
        );
    else if (ctx->compoundStatement())
        return change_node<CompoundStmt, Stmt>(
                visitCompoundStatement(ctx->compoundStatement())
        );
    else if (ctx->selectionStatement())
        return visitSelectionStatement(ctx->selectionStatement());
    else if (ctx->iterationStatement())
        return visitIterationStatement(ctx->iterationStatement());
    else if (ctx->jumpStatement())
        return visitJumpStatement(ctx->jumpStatement());
    else
        report_unhandled_case("visitStatement", ctx);
}

ast::node_ptr<ast::FuncBody> ParserVisitor::visitFunctionBody(CPMParser::FunctionBodyContext *ctx) {
    auto source_info = src_info(ctx);
    return make_node<FuncBody>(std::move(source_info),
                               visitCompoundStatement(ctx->compoundStatement()));
}

ast::node_ptr<ast::FuncDef>
ParserVisitor::visitFunctionDefinition(CPMParser::FunctionDefinitionContext *ctx) {
    auto source_info = src_info(ctx);

    // is this a constructor definition?
    bool ctor = !ctx->declSpecifierSeq();
    cpm::Type *underlying_type = ctor ? context.getSimpleType("void", false) :
                                 visitDeclSpecifierSeq(ctx->declSpecifierSeq());

    ast::node_ptr<ast::Decl> declarator = visitDeclarator(ctx->declarator(), underlying_type);
    auto body = visitFunctionBody(ctx->functionBody());

    auto *func_decl = dynamic_cast<ast::FunctionDecl *>(declarator.get());
    if (func_decl) {
        auto func_def = make_node<FuncDef>(std::move(source_info),
                                           node_ptr<FunctionDecl>(func_decl),
                                           std::move(body),
                                           ctor);
        declarator.release();
        return func_def;
    } else
        report_error("invalid function definition", ctx);
}

ast::node_ptr<ast::Expr>
ParserVisitor::visitCommaExpression(CPMParser::CommaExpressionContext *ctx) {
    auto source_info = src_info(ctx);
    if (ctx->assignmentExpression().size() == 1)
        return visitAssignmentExpression(ctx->assignmentExpression(0));
    vector<node_ptr<Expr>> exs;
    for (const auto &ae: ctx->assignmentExpression())
        exs.push_back(visitAssignmentExpression(ae));
    return make_node<CommaExpr, Expr>(std::move(source_info), std::move(exs));
}

ast::node_ptr<ast::Stmt> ParserVisitor::visitJumpStatement(CPMParser::JumpStatementContext *ctx) {
    auto source_info = src_info(ctx);
    if (ctx->Return()) {
        optional<node_ptr<Expr>> expr;
        if (ctx->commaExpression())
            expr = visitCommaExpression(ctx->commaExpression());
        return make_node<ReturnStmt, Stmt>(std::move(source_info), std::move(expr));
    } else if (ctx->Break()) {
        size_t loops = 1;
        if (ctx->IntegerLiteral())
            loops = std::stoi(ctx->IntegerLiteral()->getText());
        return make_node<BreakStmt, Stmt>(std::move(source_info),
                                          loops);
    } else if (ctx->Continue()) {
        size_t loops = 1;
        if (ctx->IntegerLiteral())
            loops = std::stoi(ctx->IntegerLiteral()->getText());

        return make_node<ContinueStmt, Stmt>(std::move(source_info), loops);
    } else
        report_unhandled_case("visitJumpStatement", ctx);
}

ast::node_ptr<ast::Stmt>
ParserVisitor::visitIterationStatement(CPMParser::IterationStatementContext *ctx) {
    auto source_info = src_info(ctx);

    if (ctx->Do() && ctx->condition() && ctx->While())
        return make_node<DoWhileStmt, Stmt>(std::move(source_info),
                                            visitCondition(ctx->condition()),
                                            visitStatement(ctx->statement()));
    else if (ctx->While() && ctx->condition() && !ctx->Do())
        return make_node<WhileStmt, Stmt>(std::move(source_info), visitCondition(ctx->condition()),
                                          visitStatement(ctx->statement()));
    else if (ctx->For()) {
        auto forInitStmt = visitForInitStatement(ctx->forInitStatement());
        std::optional<node_ptr<Condition>> cond;
        std::optional<node_ptr<Expr>> post_iter;
        auto body = visitStatement(ctx->statement());
        if (ctx->condition())
            cond = visitCondition(ctx->condition());
        if (ctx->commaExpression())
            post_iter = visitCommaExpression(ctx->commaExpression());
        return make_node<ForStmt, Stmt>(std::move(source_info), std::move(forInitStmt),
                                        std::move(cond),
                                        std::move(post_iter), std::move(body));
    } else
        report_unhandled_case("visitIterationStatement", ctx);
}

ast::node_ptr<ast::Condition> ParserVisitor::visitCondition(CPMParser::ConditionContext *ctx) {
    auto source_info = src_info(ctx);
    if (!ctx->commaExpression())

        report_unhandled_case("visitCondition", ctx);
    return make_node<Condition>(std::move(source_info),
                                visitCommaExpression(ctx->commaExpression()));
}

ast::node_ptr<ast::ForInitStmt>
ParserVisitor::visitForInitStatement(CPMParser::ForInitStatementContext *ctx) {
//    auto source_info = src_info(ctx);
    if (ctx->expressionStatement())
        return change_node<ExprStmt, ForInitStmt>(
                visitExpressionStatement(ctx->expressionStatement()));
    else if (ctx->simpleDeclaration())
        return change_node<SimpleDeclar, ForInitStmt>(
                visitSimpleDeclaration(ctx->simpleDeclaration()));
    else
        report_unhandled_case("visitForInitStatement", ctx);
}

ast::node_ptr<ast::Stmt>
ParserVisitor::visitSelectionStatement(CPMParser::SelectionStatementContext *ctx) {
    auto source_info = src_info(ctx);
    if (ctx->If()) {
        std::optional<node_ptr<Stmt>> else_body;
        if (ctx->Else())
            else_body = visitStatement(ctx->statement()[1]);
        return make_node<IfStmt, Stmt>(std::move(source_info), visitCondition(ctx->condition()),
                                       visitStatement(ctx->statement()[0]),
                                       std::move(else_body));
    } else
        report_unhandled_case("visitSelectionStatement", ctx);
}

ast::SourceInfo ParserVisitor::src_info(antlr4::ParserRuleContext *ctx) {
    if (auto start_tok = ctx->getStart())
        return SourceInfo(start_tok->getLine(), start_tok->getCharPositionInLine() + 1);
    throw std::runtime_error("src_info: got ParserRuleContext without start_tok");
}

std::vector<ast::node_ptr<ast::Expr>>
ParserVisitor::visitExpressionList(CPMParser::ExpressionListContext *ctx) {
    vector<node_ptr<Expr>> exprs;
    for (const auto &e: ctx->assignmentExpression())
        exprs.push_back(visitAssignmentExpression(e));
    return exprs;
}

cpm::SimpleType *ParserVisitor::visitTypeSpecifierSeq(CPMParser::TypeSpecifierSeqContext *ctx) {
    auto source_info = src_info(ctx);
    vector<string> t_specs;
    for (const auto &ts: ctx->typeSpecifier())
        t_specs.push_back(visitTypeSpecifier(ts));
    return getTypeFromSeq(t_specs, ctx);
}

cpm::Type *ParserVisitor::visitTheTypeId(CPMParser::TheTypeIdContext *ctx) {
    auto source_info = src_info(ctx);
    vector<bool> pointers;
    cpm::Type *type = visitTypeSpecifierSeq(ctx->typeSpecifierSeq());
    for (const auto &p: ctx->pointerOperator())
        type = context.getPointerType(type, visitPointerOperator(p));
    return type;
}

ast::UnaryOp ParserVisitor::visitUnaryOperator(CPMParser::UnaryOperatorContext *ctx) {
    switch (ctx->op->getType()) {
        case CPMParser::PlusPlus:
            return UnaryOp::PlusPlus;
        case CPMParser::MinusMinus:
            return UnaryOp::MinusMinus;
        case CPMParser::Star:
            return UnaryOp::UnStar;
        case CPMParser::And:
            return UnaryOp::UnAnd;
        case CPMParser::Plus:
            return UnaryOp::UnPlus;
        case CPMParser::Minus:
            return UnaryOp::UnMinus;
        case CPMParser::Tilde:
            return UnaryOp::BitNot;
        case CPMParser::Not:
            return UnaryOp::Not;
        case CPMParser::Sizeof:
            return UnaryOp::Sizeof;
        default:
            report_unhandled_case("visitUnaryOperator", ctx);
    }
}

ast::AssignOp ParserVisitor::visitAssignmentOperator(CPMParser::AssignmentOperatorContext *ctx) {
    auto op = ctx->op;

    switch (op->getType()) {
        case CPMParser::Assign:
            return ast::Assign;
        case CPMParser::PlusAssign:
            return ast::PlusAssign;
        case CPMParser::MinusAssign:
            return ast::MinusAssign;
        case CPMParser::StarAssign:
            return ast::StarAssign;
        case CPMParser::DivAssign:
            return ast::DivAssign;
        case CPMParser::ModAssign:
            return ast::ModAssign;
        case CPMParser::AndAssign:
            return ast::AndAssign;
        case CPMParser::OrAssign:
            return ast::OrAssign;
        case CPMParser::XorAssign:
            return ast::CaretAssign;
        case CPMParser::LeftShiftAssign:
            return ast::LeftShiftAssign;
        case CPMParser::RightShiftAssign:
            return ast::RightShiftAssign;
        default:
            report_unhandled_case("visitAssignmentOperator", ctx);
    }

}

void ParserVisitor::report_error(const string &msg, antlr4::ParserRuleContext *ctx) {
    ast::SourceInfo source_info = src_info(ctx);
    string err_msg = "line " + source_info.str() + ": error: " + msg;
    err_msg += '\n' + context.getInputByLines().at(source_info.line_no);
    throw std::runtime_error(err_msg);
}

void ParserVisitor::report_unhandled_case(const string &err_loc, antlr4::ParserRuleContext *ctx) {
    report_error("compiler error - unhandled case in: "s + err_loc, ctx);
}

void ParserVisitor::warning(const string &msg, antlr4::ParserRuleContext *ctx) {
    size_t line_no = src_info(ctx).line_no;
    string err_msg = "line " + to_string(line_no) + ": warning: " + msg;
    cout << err_msg << endl;
}

std::string ParserVisitor::visitClassName(CPMParser::ClassNameContext *ctx) {
    return ctx->Identifier()->getText();
}

std::string ParserVisitor::visitClassHeadName(CPMParser::ClassHeadNameContext *ctx) {
    return visitClassName(ctx->className());
}

ast::ClassKey ParserVisitor::visitClassKey(CPMParser::ClassKeyContext *ctx) {
    if (ctx->Class())
        return ast::Class;
    else if (ctx->Struct())
        return ast::Struct;
    else
        report_unhandled_case("visitClassKey", ctx);
}

ast::node_ptr<ast::MemberDeclarator>
ParserVisitor::visitMemberDeclarator(CPMParser::MemberDeclaratorContext *ctx,
                                     cpm::Type *underlying_type) {
    return visitDeclarator(ctx->declarator(), underlying_type);
}

ast::node_ptr<ast::MemberDeclaration>
ParserVisitor::visitMemberDeclaration(CPMParser::MemberdeclarationContext *ctx) {
    auto source_info = src_info(ctx);
    if (ctx->memberDeclaratorList()) {
        cpm::Type *underlying_type = visitDeclSpecifierSeq(ctx->declSpecifierSeq());
        return change_node<MemberDeclaratorList, MemberDeclaration>(
                visitMemberDeclaratorList(ctx->memberDeclaratorList(), underlying_type)
        );
    } else if (ctx->functionDefinition()) {
        auto fd = visitFunctionDefinition(ctx->functionDefinition());
        return change_node<FuncDef, MemberDeclaration>(std::move(fd));
    } else
        report_unhandled_case("visitMemberDeclaration", ctx);

}

ast::node_ptr<ast::MemberSpecification>
ParserVisitor::visitMemberSpecification(CPMParser::MemberSpecificationContext *ctx) {
    auto source_info = src_info(ctx);
    vector<node_ptr<MemberSpecElem>> list;
    for (const auto &elem: ctx->memberSpecElem())
        list.push_back(visitMemberSpecElem(elem));
    return make_node<MemberSpecification>(std::move(source_info),
                                          std::move(list));
}

ast::node_ptr<ast::MemberDeclaratorList>
ParserVisitor::visitMemberDeclaratorList(CPMParser::MemberDeclaratorListContext *ctx,
                                         cpm::Type *underlying_type) {
    auto source_info = src_info(ctx);
    vector<node_ptr<MemberDeclarator>> list;
    for (const auto &p: ctx->memberDeclarator())
        list.push_back(visitMemberDeclarator(p, underlying_type));
    return make_node<MemberDeclaratorList>(std::move(source_info),
                                           std::move(list));
}

ast::node_ptr<ast::ClassHead> ParserVisitor::visitClassHead(CPMParser::ClassHeadContext *ctx) {
    auto source_info = src_info(ctx);
    return make_node<ClassHead>(std::move(source_info),
                                visitClassKey(ctx->classKey()),
                                visitClassHeadName(ctx->classHeadName()));
}

ast::node_ptr<ast::ClassDef>
ParserVisitor::visitClassDefinition(CPMParser::ClassDefinitionContext *ctx) {
    auto source_info = src_info(ctx);
    auto head = visitClassHead(ctx->classHead());
    optional<node_ptr<MemberSpecification>> ms;
    if (ctx->memberSpecification())
        ms = visitMemberSpecification(ctx->memberSpecification());
    return make_node<ClassDef>(std::move(source_info),
                               std::move(head),
                               std::move(ms));
}

cpm::SimpleType *
ParserVisitor::getTypeFromSeq(const vector<string> &specs, antlr4::ParserRuleContext *ctx) {
    string type;
    bool const_ = false;
    for (const auto &ds: specs) {
        string repr = ds;
        if (repr == "const") {
            if (const_)
                report_error("multiple const qualifiers", ctx);
            const_ = true;
        } else {
            if (!type.empty())
                report_error("multiple types in one declaration", ctx);
            /* this will need refactoring if nested classes/namespaces
             * are added */
            type = repr;
        }
    }
    if (type.empty())
        report_error("missing type specification", ctx);

    return context.getSimpleType(type, const_);
}

bool ParserVisitor::valid_array_elem_type(cpm::Type *type) {
    return !is_void(type) && !cpm::function_ty(type);
}

bool ParserVisitor::is_void(cpm::Type *type) {
    cpm::SimpleType *st;
    return (st = cpm::simple_ty(type)) && st->getTypeId() == "void";
}

ast::node_ptr<ast::MemberSpecElem>
ParserVisitor::visitMemberSpecElem(CPMParser::MemberSpecElemContext *ctx) {
    auto source_info = src_info(ctx);
    if (ctx->memberdeclaration())
        return change_node<ast::MemberDeclaration, ast::MemberSpecElem>(
                visitMemberDeclaration(ctx->memberdeclaration())
        );
    else if (ctx->accessSpecifier()) {
        return change_node<ast::AccessModifier, ast::MemberSpecElem>(
                visitAccessSpecifier(ctx->accessSpecifier())
        );
    }
    report_unhandled_case("visitMemberSpecElem", ctx);
}

ast::node_ptr<ast::AccessModifier>
ParserVisitor::visitAccessSpecifier(CPMParser::AccessSpecifierContext *ctx) {
//    auto source_info = src_info(ctx);
    ast::AccessModifier am;
    if (ctx->Public())
        am = ast::PUBLIC;
    else if (ctx->Private())
        am = ast::PRIVATE;
    else
        report_unhandled_case("visitAccessSpecifier", ctx);
    return ast::make_node<ast::AccessModifier>(am);
}
